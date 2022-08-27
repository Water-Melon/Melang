### Function

Function is used to encapsulate a series of calculations and algorithms.

There are two mode:

- function definition
- function call



#### Definition

Syntax:

```
@funcname (parameters) {
  ...//statements
}
```

*funcname* is the name of function, it has to follow the rule:

> start with letter or underscore and the rest part can be number, letter or underscore.

*parameters* is a series of variables, like: *name*, *age* or anything you need.

e.g.

```
@plus(a, b)
{
  return a+b;
}
```

Here is a function named *plus*. It has two parameters named *a* and *b*. And its function is to calculate the sum of two parameters.



#### Call

Syntax:

```
funcname(arguments);
```

*funcname* is the name of defined function.

*arguments* is a series of values.

e.g.

```
plus(1, 2);
```

*plus* is the *funcname*, and 1, 2 correspond to parameter *a* and *b*.



#### Reference

Let's see an example at first.

```
@foo(data)
{
  return data + 100;
}

a = 0;
a = foo(a);
```

This program just modified *a* from 0 to 100, and that is what we wish it to be.

But the last statement is a little bit clumsy. How to make it better?

```
@foo(&data)
{
  data += 100;
}

a = 0;
foo(a);
```

We change function parameter *data* to be *&data*.

Operator *&* makes argument *a* to be a reference variable named *data* in function *foo*. Every modification on *data* will directly effect on *a*.

> Note:
>   1. & only can be used on the parameters of function definition.
>   2. array and object will be treated as reference all the time no matter & given or not.



#### Lack of arguments

```
@foo(data)
{
  dump(data);
}

foo();
```

This program will not report error. Because interperter will set *nil* to *data* automatically.



#### Return function

Let's see an example:

```
@foo() {
  @bar() {
    sys = _import('sys');
    sys.print('bar');
  }
  return bar;
}

@foo()();
```

Guess what happened?

'bar' will be printed on terminal.

It is possible because function definition is a statement. So we can define a function in another function's definition.



#### Variable arguments

In most programming languages, they all support variable arguments. In melang, we have an alternative plan.

```
@log()
{
  sys = _import('sys');

  s = 'argument list: ';
  for (i = 0; i < sys.size(args); ++i) {
      s += args[i] + ' ';
  }
  sys.print(s);
}

log('this', 'is', 'a', 'variable', 'arguments', 'example');
```

The result of this program is:

```
argument list: this is a variable arguments example 
```

The key point is variable *args*. It's a internal variable in Melang function.

It is an array to record every arguments passed to this function.

> Function *sys.size* returns the number of array elements.



#### Closure

Let us see an example at first.

```
@foo()
{
  a = 1;
  @bar()
  {
    sys = _import('sys');

    sys.print(a);
  }
  return bar;
}

b = foo();
b();
```

In this example, I want to output the value of `a` which is defined in function `foo` in function `bar`. Obviously, the output is `nil`, because `a` can not be found in function `bar`.

But there is a way to get `a` in `bar`.

```
@foo()
{
  a = 1;
  @bar() $(a)
  {
    sys = _import('sys');
    sys.print(a);
  }
  return bar;
}

b = foo();
b();
```

Now, the output is `1`.

Let us modify this example again:

```
@foo()
{
  a = 1;
  @bar() $(a)
  {
    sys = _import('sys');
    sys.print(a);
  }
  a = 100; //change to another value
  return bar;
}

b = foo();
b();
```

Now, the output still be `1`. How can I do if I want the value `a` in function `bar` is the modified value (`100`)?

```
@foo()
{
  a = 1;
  @bar() $(&a) //to become a reference
  {
    sys = _import('sys');
    sys.print(a);
  }
  a = 100; //change to another value
  return bar;
}

b = foo();
b();
```

