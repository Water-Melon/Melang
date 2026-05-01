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

sys.print('all tests passed');
