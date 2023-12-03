### Reactive programming

Derive from web front-end programming, Melang also supports reactive programming.

Reactive programming is based on two functions:

- watch

  ```
  Watch(var, func, userdata);
  ```

- unwatch

  ```
  unWatch(var);
  ```

But these functions are bound with interpreter tightly, so they won't be achieved in library.

Calling `watch` to trace a variable, a callback function will be called after the variable's value changed.

e.g.

```
@handler (newValue, userData)
{
  sys = Import('sys');
  sys.print(newValue);
  sys.print(userData);
  userData = 'world';
}
a = 10;
userData = 'hello';
Watch(a, handler, userData);
a = 11;
a = 12;
```

The result of this code is:

```
11
hello
12
hello
```

As we can see in this example, if `watch` is called, `handler` will be called when `a`'s value changed.

We should note that `handler` is a function, its parameter supports reference. In this example, `newValue` and `userData` are not references. So every modification on `userData` can not affect on the outer scope variable as we wish.

So let's fix this problem.

```
@handler (newValue, &userData)
{
  sys = Import('sys');
  sys.print(newValue);
  sys.print(userData);
  userData = 'world';
}
a = 10;
userData = 'hello';
Watch(a, handler, userData);
a = 11;
a = 12;
```

Now, the output is:

```
11
hello
12
world
```



If we don't want to trace this variable any more, we can use `Unwatch` to stop tracing.

```
Unwatch(a);
a = 13;
```

`handler` won't be called any more, and nothing will be printed on terminal.
