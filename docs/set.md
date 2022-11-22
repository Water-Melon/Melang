### Set

Syntax:

```
setname {
  property1;
  ...//some properties
  @method1(parameters) {
    ...//statements
  }
  ...//some methods
}
```

Actually set is very like structure in C. It has property definition, but Melang also supports method definition in Set.

Property in Set is a kind of variable. Method is a kind of function.

e.g.

```
human {
  name;
  age;
  gender;
  @init (name, age, gender)
  {
    this.name = name;
    this.age = age;
    this.gender = gender;
  }
}
```

*human* is a set name. It has some properties: *name, age, gender, score*. And one method: *init*.

In this example, you can see:

- property definition just need a name
- method definition has a special local variable named *this*

*this* is an internal variable in method. It is a reference to set's object.



Now we have two questions:

1. What is object
2. What a set definition can do

Set is an abstract definition and object is a instantiated Set.

As shown above, set *human* is a definition. It defines some basic attributes of a human.

So if there is a boy named Tom, and he also has these attributes, then he is an instance of set *human*, which means Tom is an object of set *human*.

So how to instantiate a set object?

```
Tom = $human;
```

That's done. Now, we have a variable named *Tom*, and that is an object of set *human*.

Now, we can visit *Tom*'s properties and methods.

```
sys = Import('sys');

Tom.init('Tom Moore', 8, 'male');

sys.print(Tom.name);
```

The first line is calling method *init*, this method initiates object *Tom*'s properties.

The last line outputs *Tom*'s name.

And we can see, if we want to visit an object's property or method, we need to use operator *.* (dot). Left of operator . is an object and right is object's property or method.

Back to the topic *this*. In this example, *this* is a reference of object *Tom*. So modifications on *this* will directly affect on *Tom*. So the result of the last one statement is 'Tom Moore'.
