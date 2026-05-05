/*
 * Smoke-test for Melang. Exercises:
 *   arithmetic, comparisons, recursion, loops, arrays, strings,
 *   closures (with $(&) capture), Set/objects, Watch/Unwatch, Eval.
 * Each section asserts (); failure aborts.
 */
sys = Import('sys');

@Assert(cond, msg)
{
  if (!cond) {
    sys.print('ASSERT FAILED: ');
    sys.print(msg);
    Dump(cond);
    sys.exit(1);
  } fi
}

/* arithmetic */
Assert(1 + 2 == 3, 'add');
Assert(10 - 4 == 6, 'sub');
Assert(3 * 5 == 15, 'mul');
Assert(20 / 4 == 5, 'div');
Assert(17 % 5 == 2, 'mod');
Assert(-3 + 5 == 2, 'unary minus');

/* comparisons + logical */
Assert(1 < 2, 'lt');
Assert(2 <= 2, 'le');
Assert(3 > 1, 'gt');
Assert(3 >= 3, 'ge');
Assert(1 == 1, 'eq');
Assert(1 != 2, 'ne');
Assert((1 == 1) && (2 == 2), 'and');
Assert((1 == 0) || (2 == 2), 'or');

/* recursion */
@F(i) {
  if (i <= 2) {return 1;} fi
  return F(i - 1) + F(i - 2);
}
Assert(F(10) == 55, 'fib(10)');
Assert(F(15) == 610, 'fib(15)');

/* loop + scope */
sum = 0;
i = 0;
while (i < 10) {
  sum = sum + i;
  i = i + 1;
}
Assert(sum == 45, 'sum 0..9');

prod = 1;
for (j = 1; j <= 5; j = j + 1) {
  prod = prod * j;
}
Assert(prod == 120, 'factorial 5');

/* arrays */
arr = ['a', 'b', 'c', 'd'];
Assert(arr[0] == 'a', 'array idx 0');
Assert(arr[3] == 'd', 'array idx 3');
arr[4] = 'e';
Assert(arr[4] == 'e', 'array append');

m = ['name': 'kong', 'count': 7];
Assert(m['name'] == 'kong', 'string key 1');
Assert(m['count'] == 7, 'string key 2');

/* strings */
s = 'hello';
Assert(s == 'hello', 'string eq');

/* closure ($(&n) captures by reference) */
@make_counter() {
  n = 0;
  @bump() $(&n) {
    n = n + 1;
    return n;
  }
  return bump;
}
c = make_counter();
Assert(c() == 1, 'closure 1');
Assert(c() == 2, 'closure 2');
Assert(c() == 3, 'closure 3');

/* reference parameter */
@inc(&v) {
  v = v + 100;
}
x = 1;
inc(x);
Assert(x == 101, 'ref param');

/* mixed-ref / by-value parameter declaration.  Regression test for the
 * funcdef_args_get type-leak bug: when the first declared parameter is
 * &x, subsequent y, z were silently created as REFER and aliased back
 * to the caller, so `b` and `c` were modified through y, z.  After the
 * fix, only `a` (declared with &) propagates. */
@mix(&a, b, c) {
  a = a + 100;
  b = b + 100;
  c = c + 100;
}
ma = 1; mb = 2; mc = 3;
mix(&ma, mb, mc);
Assert(ma == 101, 'mixed ref decl: a propagates');
Assert(mb == 2,   'mixed ref decl: b stays by-value');
Assert(mc == 3,   'mixed ref decl: c stays by-value');

/* Set / object */
Point {
  x;
  y;
}
p = $Point;
p.x = 3;
p.y = 4;
Assert(p.x == 3, 'set field x');
Assert(p.y == 4, 'set field y');

/* Watch / Unwatch */
@on_change(newv, &acc) {
  acc = acc + newv;
}
acc = 0;
target = 0;
Watch(target, on_change, acc);
target = 10;
target = 20;
target = 30;
Unwatch(target);
target = 999;
Assert(acc == 60, 'watch accumulator');

/* Eval */
Eval('y = 7 + 3;', nil, true);

/* Tagged-value operand stack regressions (high-cost-tier rework).
 * These deliberately stress paths where the new tagged stack
 * differs from the previous boxed-only stack:
 *   - LOAD_LOCAL pushes a borrow rather than ref-counting.
 *   - DUP copies the slot and stamps borrow=1.
 *   - Hot int+int arithmetic stays unboxed end-to-end.
 *   - Mixed int/real falls through value_take_var to apply_binop.
 *   - String slots stay boxed; pass-through must not double-decref.
 */
@TV_dup_chain() {
  /* a -> b -> c each LOAD_LOCAL the previous slot as a borrow. */
  a = 5; b = a; c = b;
  return a + b + c;
}
Assert(TV_dup_chain() == 15, 'tagged-value DUP/borrow chain');

@TV_postfix() {
  /* Postfix ++ pushes the OLD value of the slot as an unboxed int
   * while the slot itself increments — exercises the borrow path on
   * the old-value side of LOAD_LOCAL_INC. */
  i = 10;
  r = i++ + i;
  return r;
}
Assert(TV_postfix() == 21, 'tagged-value postfix++');

@TV_mixed_arith() {
  /* int + real exercises the slow path: both operands materialize
   * via value_take_var, apply_binop runs as before. */
  return 1 + 2.5;
}
Assert(TV_mixed_arith() == 3.5, 'tagged-value int+real slow path');

@TV_string_passthrough() {
  /* String slots stay boxed under the tagged stack. Three pass-
   * throughs and a concat must not over-decref. */
  s = 'abc';
  t = s;
  u = t;
  return u + 'def';
}
Assert(TV_string_passthrough() == 'abcdef', 'tagged-value string pass-through');

@TV_array_sum() {
  /* GET_INDEX returns an unboxed int; the running sum stays unboxed
   * until it eventually escapes into the return path. */
  a = [1, 2, 3, 4, 5];
  s = 0;
  for (i = 0; i < 5; i = i + 1) {
    s = s + a[i];
  }
  return s;
}
Assert(TV_array_sum() == 15, 'tagged-value array sum');

/*
 * Top-level / function-body VM-to-AST fallback regression.
 *
 * Function bodies whose loop nesting exceeds the VM compiler's bound
 * (MLN_VM_MAX_LOOPS=16 in mln_lang_vm.c) bail out of vm_try_compile
 * and run on the AST stack-walker instead.  Top-level code with the
 * same shape used to crash the script with "VM: top-level cannot be
 * compiled (internal error)"; a Melon-side fix flips
 * ctx->vm_use_ast=1 and lets the AST walker drive ctx->stm
 * transparently.  We exercise the function-body fallback path here
 * — this is the case Melang utility code hits most often.
 */
@deep_loops() {
  /* 17 nested while(1){...break;} loops force vm_state == -1; the
   * AST fallback then runs the body and returns 'ok'. */
  while (1) {
  while (1) {
  while (1) {
  while (1) {
  while (1) {
  while (1) {
  while (1) {
  while (1) {
  while (1) {
  while (1) {
  while (1) {
  while (1) {
  while (1) {
  while (1) {
  while (1) {
  while (1) {
  while (1) { return 'ok'; }
  break; }
  break; }
  break; }
  break; }
  break; }
  break; }
  break; }
  break; }
  break; }
  break; }
  break; }
  break; }
  break; }
  break; }
  break; }
  break; }
  return 'unreachable';
}
Assert(deep_loops() == 'ok', 'deep-loops AST fallback');

sys.print('all tests passed');
