### Data types

Melang is a implicit-type script language. So you don't have to give variable's type in code.

But every value has its type. Types in Melang are shown below:

- integer
- real
- boolean
- string
- nil
- array
- set
- object
- function



e.g.

```
// this is a comment line (start with two slashs)
/*...*/ this is a comment block (start with /* end with */)

//variable i has an integer value
i = 1;

//f has a real value
f = 1.2;

//b has a boolean value
b = true;

//n's value is nil, and nil means none
n = nil;

//[1, 2] is an array with two elements (1 and 2)
a = [1, 2];

//the value of d is a dict but in melang dict is a kind of array
d = ['age': 18, 'name': 'Mr. M'];

//human is a set, it's equivalent to Structure in C, but it supports function definition
human {
  name;
  age;
  @run () { // this is a method definition.
    //...
  }
}

//value of o is an object or instance of set human.
o = $human;

//access object properties
o.name;
o.age = 1;

//accessing non-existent object properties
o.gender // nil will be given
o.phone_number = 123; // the value of the new property `phone_number` has been set to 123.

//here is a function definition
@foo()
{
  //...
}

// and here is a function call
foo();
```



##### Array

Array is a kind of data set. If we want to get or access its elements, we can do it as shown below:

```
a = [1, 2, 3];

a[0]; //array[index]
```

`a[0]` retrieves the first element `1` from the array `a`, while `a[2]` retrieves the last element `3`.

If the index is greater than or equal to the array length, `nil` will be given.



There is another kind of array named *dict*.

```
d = ['name': 'Tom', 'age': 18];
```

Now if we want to access the element which *key* is `name`, we can do it in this way:

```
d['name']; //dict[key]
```

If *key* is not in this dict,  `nil` will be given.
