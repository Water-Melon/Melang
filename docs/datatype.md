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

i = 1; //variable i has an integer value
f = 1.2; //f has a real value
b = true; //b has a boolean value
n = nil; //n's value is nil, and nil means none
a = [1, 2]; //[1, 2] is an array with two elements (1 and 2)
d = ['age': 18, 'name': 'Mr. M']; //the value of d is a dict but in melang dict is a kind of array
human {
  name;
  age;
} //human is a set, it's equivalent to Structure in C, but it supports function definition
o = $human; //value of o is an object or instance of set human.
```

