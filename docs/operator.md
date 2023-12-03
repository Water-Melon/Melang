### Operators

Melang supports these operators as shown below:

| operators                                    | combination direction |
| -------------------------------------------- | --------------------- |
| ,                                            | left to right         |
| =  +=  -=  >>=  <<=  *=  /=  \|=  &=  ^=  %= | right to left         |
| && \|\|                                      | left to right         |
| &  \|  ^                                     | left to right         |
| ==  !=                                       | left to right         |
| \>  >=  <  <=                                | left to right         |
| \>>  <<                                      | left to right         |
| \+  -                                        | left to right         |
| \*  /  %                                     | left to right         |
| !                                            | -                     |
| ++  --  (suffix)                             | -                     |
| []  .                                        | left to right         |
| \-  ~  &  $  ()  ++  -- (prefix)             | -                     |

These operators are listed by priority from low to high.



### Examples

Here, it is assumed that you have already installed Melang and all its library files (installed using the `make all` command).

The `sys.print` will output the contents of the parameter to the terminal.

```
sys = Import('sys');

i = 1, 2, 3;
sys.print(i); // 1

i += 1;
sys.print(i); // 2

b = i == 2; //b is a boolean value
sys.print(b); // true

i = i | 1;
sys.print(i); // 3

sys.print(i++); // 3

sys.print(++i); // 5

sys.print(i--); // 5

sys.print(--i); // 3

sys.print(i == 3 && 100 || 50); // 100

human { // set definition
  name;
  age;
}
friend = $human; //instantiate a human object
friend.name = 'Alex';
friend.age = 18;
sys.print(friend.name); // Alex
sys.print(friend.age); // 18

sys.print(!friend.age); //false

arr = [1, 2, 3];
sys.print(arr[2]); // 3
```

The output is:

```
1
2
true
3
3
5
5
3
100
Alex
18
false
3
```

