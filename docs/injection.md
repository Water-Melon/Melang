### Injection

Injection is for Object. It allows us to add many properties and methods in any object.

```
Human {
}
someone = $Human;
```

Now, we have a set *Human* without any property or method, and an object of *Human* named *someone*.

So as a human, *someone* must has age. Let's add for it.

```
someone.age = 20;
@mln_print(someone.age);
```

OK, done. Now *20* will be printed on terminal.

Let's add a method for *someone*.

```
@printAge()
{
  @mln_print(this.age);
}

someone.print = printAge;
someone.print();
```

*printAge* is a function not object method, but it can be treated as an object method by injection. In a common function, *this* is *nil*. So if call *printAge* directly, interpreter will throw an error.



There is another way to inject properties and methods.

##### mln_set_property

```
@mln_set_property(object, name, value);
```

use *mln_set_property* to inject property or method into an object. Its return value is the same as argument *value*.

Let's add a *name* in *someone*.

```
ret = @mln_set_property(someone, 'name', 'Jason');
@mln_print(ret);
@mln_print(someone.name);
```

The output of this piece of code is:

```
Jason
Jason
```

Let's add a method in *someone* to print name.

```
@printName()
{
  @mln_print(this.name);
}
@mln_set_property(someone, 'showName', printName);
someone.showName();
```



Since there is a function for setting property, there will be a function for getting property.

##### mln_get_property

```
@mln_get_property(object, name);
```

e.g.

```
@mln_print(@mln_get_property(someone, 'name'));
```

The output is:

```
Jason
```

But if you try to access a nonexistent property by this function, interperter will throw an error.
