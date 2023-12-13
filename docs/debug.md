### Debugging



#### Call Stack

We can use the function  `Stack` to output the call stack information of the current coroutine.

```
Stack();
```

Currently, tracing of function names set by the function `Watch` is **not** supported.

##### Example

```
@foo() {
  Stack();
}

foo();
```

The output is:

```
1: foo a.m:2
2: __main__ a.m:5
```

The name of the function is not accurate because in Melang, everything is a variable, and the definition of a function is only the value of a variable. Therefore, the function name output here is actually the variable name. However, the subsequent file and line numbers are accurate.



#### Dump

We can use the function `Dump` to output the information of the given variable.

```
Dump(var);
```



##### Example

```
Human {
  run;
}

@run() {
  return 'running...';
}

Tom = $Human;
Tom.run = run;
Dump(Tom.run);
```

The output is:

```
  var <Var>    Refer  Alias name: var  valueRef: 2, udata <0x0>, func <0x0>, not_modify: false
    <FUNCTION> a.m:6 NARGS:0
```

`Dump` can output the name and line number of the file where the first statement of the currently invoked function is located. This is very helpful for us to trace function calls within certain complex structures.