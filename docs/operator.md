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
| ++  --  (suffix)                             | -                     |
| []  .                                        | left to right         |
| \-  ~  !  &  $  ()  ++  -- (prefix)          | -                     |

These operators are listed by priority from low to high.



e.g.

Try to execute these statements, and guess their results.

```
i = 1, 2, 3; //guess i = ?, you can use function mln_print to output on your terminal
@mln_print(i);

i += 1; //increase 1

b = i == 2; //b is a boolean value

i = i | 1; //guess i = ?

i++;

--i;

//Instantiate a human object
human {
  name;
  age;
}
friend = $human; //friend is an object
friend.name = 'Alex';
friend.age = 18;
```

