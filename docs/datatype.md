### Data types

Melang is a implicit-type script language. So you don't have to give variable's type in code.

But every value has its type. Types in Melang are shown below:

- integer
- real
- boolean
- nil
- array
- set
- object



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
}

//value of o is an object or instance of set human.
o = $human;
```

