### Flow control

Flow control statement including:

- if-else
- for
- while
- switch
- goto
- break
- continue
- return



#### if-else

Syntax:

```
//mode A
if (conditions) {
  ...//some statements
} fi

//mode B
if (conditions) {
  ...//some statements
} else {
  ...//some statements
} fi
```

`conditions` is one or more expressions.

The boolean value of `conditions` is not only including `true` and `false`, but also including any other values. So there are some values will be treated as `false`:

- `nil`
- `0`
- `''` (null string)
- `[]` (empty array)

In mode A: if conditions are satisfied (boolean value is *true*), interpreter will execute statements in `if` block.

In mode B: if conditions are satisfied, interpreter will execute statements in `if` block, otherwise `else` block statements will be executed.

> if there is only one statement in *if* or *else* block, the curly brackets around statement can be omitted.
>
> In mode B, there is no *fi* at the end of the statement block.

There is a special case -- `if` - `else if`, e.g.

```
if (a == 1)
  a = 2;
else if (a == 10)
  a = 8;
fi
```

Actually it just a combination of mode B and mode A. The last `if` is a single statement, so we can omit curly brackets.

e.g.

```
if (a == 1) {
  a = 2;
} fi

if (a == 1)
  a = 2;
fi

if (a == 1) {
  a = 2;
} else {
  a = 3;
}

if (a == 1)
  a = 2;
else
  a = 3;

if (a == 1) {
  a = 2;
} else if (a == 10) {
  a = 8;
} fi

if (a == 1) {
  a = 2;
} else if (a == 10) {
  a = 8;
} else {
  a = 1;
}

if (a == 1)
  a = 2;
else if (a == 10)
  a = 8;
fi

if (a == 1)
  a = 2;
else if (a == 10)
  a = 8;
else
  a = 1;
```



#### for

Syntax:

```
for (expr1; expr2; expr3) {
  ...//some statements
}
```

Interpreter will follow these steps to execute:

1. execute `expr1` (expression).
2. execute `expr2` and check its boolean value. if it is *truthy*, go to step3, otherwise go to step5.
3. executed statements in `for` block.
4. executed `expr3`, and go to step 2.
5. `for` statement finished.

> If there is only one statement in *for* block, the curly brackets around statement can be omitted.

e.g.

```
sys = Import('sys');

array = [1, 2, 3, 4];
for (i = 0; i < 4; i++)
  sys.print(array[i]);
```



#### while

Syntax:

```
while (conditions) {
  ...//some statements
}
```

`conditions` is one or more expressions.

Interpreter will follow these steps to execute:

1. test `conditions`, if its boolean value is `true`, go to step 2, otherwise go to step 3.
2. execute statements in `while` block and then go to step 1.
3. `while` statement finished.

> If there is only one statement in *while* block, the curly brackets around statement can be omitted.

e.g.

```
i = 0;
while (i < 10) {
  i++;
}
```



#### switch

Syntax:

```
switch (expr) {
  case value1:
    ...//some statements
  case value2:
    ...//some statements
  ...
  default:
    ...//some statements
}
```

Interpreter will follow these steps:

1. execute `expr` and record its value in interpreter.
2. use `expr`'s value to match every case value. If matched (two values are same), go to step 3, otherwise go to step 2 to keep matching. If no case matched, and there is a `default` case in `switch`, the `default` will be matched, then go to step3. Otherwise go to step 5.
3. executed statements those in matched case.
4. keep executing next case statements but NOT to match case value. If there is any one case (including `default`) behind current one, go to step 4.
5. *switch* statement finished.

e.g.

```
i = 'hello';
switch (i) {
  case 1:
    i = 100;
    break;
  case 'hello':
    i = 'world';
  default:
    i += ' hello';
    break;
}
```

`break` statement which will be talked soon makes interpreter stop running in `switch`.



#### goto

Syntax:

```
goto label;
```

The `goto` statement tells the interpreter to jump to the specified position marked by the `label`, and continue execution from that position.

e.g.

```
  sys = Import('sys');

  i = 10;
  goto plus;
  i--;
plus:
  i += 2;
  sys.print(i);
```

*plus* is a label. The output of this code is `12` which means `i--;` is not executed.

> `goto` in Melang is simplified.

```
@foo()
{
  sys = Import('sys');

  i = 0;
  if (++i > 10) {
l1:
    sys.print(i);
  } fi
  goto l1;
}

foo();
```

The interpreter will throw an error that `l1` can not be found, because it is not in the outest statements in a function. The next example is working correctly.

```
@foo()
{
  sys = Import('sys');

  i = 0;
li:
  if (++i > 10) {
    sys.print(i);
  } fi
  goto l1;
}

foo();
```



#### break

`break` is a single statement. It is used to interrupt current loop (including `switch`), and jump out of the loop.

e.g.

```
sys = Import('sys');

for (i = 0; i < 1000; i++) {
  if (i > 10)
    break;
  fi
}
sys.print(i);
```

The output is `11`, because when `i` greater than `10`, `if` condition is matched. Then `break` statement makes `for` loop stop and go to execute next statement (`sys.print(i);`).



#### continue

`continue` statement is also used in a loop (**except** `switch`).

When interpreter encounter `continue` statement, the rest statements those behind `continue`, will not be executed, and go back to the beginnig of loop statements.

If `continue` is encountered in a  `for` loop, `expr3` and `expr2` will be executed and then back to the beginning of loop statements or finish loop (boolean value of `expr2` is `false`).

e.g.

```
j = 0;
for (i = 0; i < 1000; i++) {
  if (i % 2) continue; fi
  ++j;
}
```



#### return

Syntax:

```
return value;
```

Usually, `return` is used in a function. It will stop interpreter executing rest statement and return a value to the caller (outer scope).

`value` can be omitted, then the return value will be `nil`.

If `return` appears in the outest scope, interpreter will stop execution, and this script task will be removed.

e.g.

```
@foo ()
{
  return 1;
}

@bar ()
{
  return; //nil
}

return; //task finished here

a = 1; //never be executed.
```
