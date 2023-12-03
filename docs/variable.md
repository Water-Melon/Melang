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
  sys = Import('sys');
  sys.print(a); //output a's value
}
foo(); //call function foo
```

The result is:

```
nil
```

 That is reasonable because interpreter just search `a` in current scope (in function *foo*), and it is not defined in this scope.

If we need to obtain the value of a global variable, then the global variable's name should begin with a capital letter.

```
A = 1; // global variable A
@foo() {
  sys = Import('sys');
  sys.print(A);
}
foo();
```

Result is:

```
1
```



Let's see another example:

```
A = 1;
b = 2;
@foo() {
  sys = Import('sys');

  A = 10;
  sys.print(A); //will output 10
  sys.print(b); //will output nil
}
```
