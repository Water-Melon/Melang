### Reflection

Reflection makes us call a specified function or set by a name string.



##### Function reflection

```
@foo ()
{
  sys = Import('sys');
  sys.print('foo');
}
a = 'foo';
a();
```

As we can see, variable `a` is a string but a function, but its value is a function name. The last statement will output `foo` on terminal, because function `foo` is called.

Let's see a more complex example:

```
@foo ()
{
  sys = Import('sys');
  sys.print('foo');
}
a = 'foo';
b = 'a';
b();
```

This example will output `foo` on terminal either.



##### Set reflection

```
sys = Import('sys');

human {
  name;
  age;
}
s = 'human';
Tom = $s;
sys.print(Tom);
```

`s` is a string variable, and its value is a set name.

The output is `Object`. Which means, `Tom = $s;` is working. `Tom` is an object of set `human`.



##### Method reflection

If function has reflection, so does method.

Let's see a comprehensive example:

```
human {
  action;
  @run ()
  {
    sys = Import('sys');
    sys.print('running');
  }
}
s = 'human';
Tom = $s;

Tom.action = 'run';
Tom.action();
```

The result of this program is:

```
running
```

The first part is a set reflection to instantiate an `human` object. And set object's `action` to be a string which is the method name `run`. And then call object method via object property.

> Note: The code shown below is not working.
>
> ```
> s = 'run';
> Tom.s();
> ```
