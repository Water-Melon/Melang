### statement

Statement is one of basic syntax in Melang. 

e.g.

```
a = 1;
b = 2;

@foo() {
  return 1;
}
```

In this example, first line is a statement, and second one is a statement too. And in Melang, function definition is also a kind of statement. And of course, *return 1;* in function *foo* is a statement either.

> Every statement must be separated by  *;* (semicolon) . Any statement must be ended by  *;*  . There are two exceptions — the definitions of function and set.

```
a = 1, c = 3;
b = 2;
@c() {
  return 1;
}
@d {
  p = 1;
  @q () {
    return 2;
  }
}
```

So we almost say that program is composed of statements.

But as we can see the first line of above example, *a = 1, c = 3;* seems not a simple stuff, it is composed of *a = 1, c = 3* and *;*.

So what constitutes a statement？



##### expression

Expression is the basic unit of statement.

Usually, one statement only has one expression. But if one statement has many expressions, every expression must be separated by  *,* (comma).

```
a = 1

a = 1, b = 2
```



So we can see, statement is composed of expressions and a semicolon, and each expression separated by a comma.

And we have to know that flow control (we will talk about that on subsequent pages) is also a kind of statement.

