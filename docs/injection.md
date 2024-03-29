### Injection

Injection is for Object. It allows us to add many properties and methods in any object.

```
Human {
}
someone = $Human;
```

Now, we have a set `Human` without any property or method, and an object of `Human` named `someone`.

So as a human, `someone` must has age. Let's add for it.

```
sys = Import('sys');
someone.age = 20;
sys.print(someone.age);
```

OK, done. Now `20` will be printed on terminal.

Let's add a method for `someone`.

```
@printAge()
{
  sys = Import('sys');
  sys.print(this.age);
}

someone.print = printAge;
someone.print();
```

`printAge` is a function not object method, but it can be treated as an object method by injection. In a regular function, `this` is `nil`. So if call `printAge` directly, interpreter will throw an error.



There is another way to inject properties and methods.

##### setter

```
sys = Import('sys');
sys.setter(object, name, value);
```

use `sys.setter` to inject property or method into an object. Its return value is the same as argument `value`.

Let's add a `name` in `someone`.

```
sys = Import('sys');

ret = sys.setter(someone, 'name', 'Jason');
sys.print(ret);
sys.print(someone.name);
```

The output of this piece of code is:

```
Jason
Jason
```

Let's add a method in `someone` to print name.

```
Sys = Import('sys');

@printName()
{
  Sys.print(this.name);
}
Sys.setter(someone, 'showName', printName);
someone.showName();
```



Since there is a function for setting property, there will be a function for getting property.

##### getter

```
sys = Import('sys');

sys.getter(object, name);
```

e.g.

```
sys = Import('sys');

sys.print(sys.getter(someone, 'name'));
```

The output is:

```
Jason
```

But if you try to access a non-existent property by this function, interperter will throw an error.
