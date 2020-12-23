### Variable

Variable is a cell to record value which may be modified at any time. And cell needs a name.

So name should be followed by rules: 

> start with letter or underscore and the rest part can be number, letter or underscore.

e.g.

```
_    _name    abc123      //correct
123  123abc   //incorrect
```

Now, let's define some variables.

```
i = 1;
name = 'Mr. M';
```



In Melang, variable name is only searched in current scope.

So if we define a variable and want to use it in a function, an unexpected result will be occurred.

e.g.

```
a = 1;
@foo() { //function definition
  @mln_print(a); //output a's value
}
@foo(); //call function foo
```

The result is:

```
nil
```

 That is reasonable because interpreter just search *a* in current scope (in function *foo*), and it is not defined in this scope.

But if we need a's value in foo, how to do?

```
a = 1;
@foo() {
  @mln_print(_a); //_a instead of a
}
@foo();
```

Result is:

```
1
```

So here is the special rule:

> If variable name start with _ (underscore), that variable whose name is the part except first underscore will be searched from current scope to outest scope.



Let's see another example:

```
a = 1;
b = 2;
@foo() {
  a = 10;
  @mln_print(a); //will output 10
  @mln_print(_a); //also 10
  @mln_print(b); //will output nil
  @mln_print(_b); //will output 2
}
```
