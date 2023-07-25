### String



### Import

```
str = Import('str');
```



##### bin2hex

Binary bytes string to hex string.

```
str.bin2hex(bin);
```

Input:

- bin - a binary bytes string.

Return value:

- a hex string.

Example:

```
str = Import('str');
sys = Import('sys');
rc = Import('rc');

sys.print(str.bin2hex(rc.rc4('HI', 'key')));
```

Output:

```
4325
```



##### hex2bin

Hex string to binary bytes.

```
str.hex2bin(hex);
```

Input:

- hex - a hex string.

Return value:

- a binary bytes string.

Example:

```
str = Import('str');
sys = Import('sys');

sys.print(str.hex2bin('4849'));
```

Output:

```
HI
```



##### bin2int

Binary bytes to an integer.

```
str.bin2int(bin);
```

Input:

- bin - a binary bytes string.

Return value:

- an integer.

Example:

```
str = Import('str');
sys = Import('sys');

sys.print(str.bin2int('HI'));
```

Output:

```
18505
```



##### int2bin

Integer to binary bytes.

```
str.int2bin(i);
```

Input:

- i - an integer.

Return value:

- a binary bytes string.

Example:

```
str = Import('str');
sys = Import('sys');

sys.print(str.int2bin(18505));
```

Output:

```
HI
```



##### bin2real

Binary bytes to real number.

```
str.bin2real(bin);
```

Input:

- a binary bytes string.

Return value:

- a real number.

Example:

```
sys.print(str.bin2real('HI'));
```

Output:

```
1070435769529469910793714477087121352287059968.000000
```



##### real2bin

Real number to binary bytes.

```
str.real2bin(r);
```

Input:

- a real number.

Return value:

- a binary bytes.

Example:

```
str = Import('str');
sys = Import('sys');

sys.print(str.real2bin(1070435769529469910793714477087121352287059968.000000));
```

Output:

```
HI
```



##### b2s

Copy binary bytes into a string.

```
str.b2s(bin);
```

Input:

- a value whose type should be one of:
  - int
  - real
  - bool

Return value:

- a string value.

Example:

```
str = Import('str');
sys = Import('sys');

sys.print(str.b2s(100));
```

Output:

```
d
```



##### s2b

String to a given type value.

```
str.s2b(s, type);
```

Input:

- s - a string.
- type - a string whose value should be:
  - int
  - real
  - bool

Return value:

- an integer or real or bool value.

Example:

```
str = Import('str');
sys = Import('sys');

sys.print(str.s2b('d', 'int'));
```

Output:

```
100
```



##### strlen

Calculate string length.

```
str.strlen(s);
```

Input:

- a string value.

Return value:

- an integer string length.

Example:

```
sys.print(str.strlen('this is a test'));
```

Output:

```
14
```



##### split

Return part of a string.

```
str.split(s, offset, len);
```

Input:

- s - a string value.
- offset - an integer offset.
- len - is optional. It should be a positive number or zero. If not set, *len* will be set as *string length - offset*.

Return value:

- The splitted string.

Example:

```
str = Import('str');
sys = Import('sys');

sys.print(str.split('this is a test', -10));
```

Output:

```
 is a test
```



##### slice

Split a string by a set of characters.

```
str.slice(s, seps);
```

Input:

- s - the input string.
- seps - a set of characters, a string.

Return value:

- Returns an array of strings created by splitting the *s* parameter on boundaries formed by the *seps*.

Example:

```
str = Import('str');
sys = Import('sys');

sys.print(str.slice('this is a test', ' s'));
```

Output:

```
[thi, i, a, te, t, ]
```



##### kmp

Find the first occurrence of a string by KMP algorithm.

```
kmp(s, pattern);
```

Input:

- s - the input string.
- pattern - the matched string.

Return value:

- the first occurrence offset on success, otherwise *nil* returned.

Example:

```
str = Import('str');
sys = Import('sys');

sys.print(str.kmp('this is a test', 'is'));
```

Output:

```
2
```



##### strstr

Find the first occurrence of a string

```
str.strstr(s, pattern);
```

Input:

- s - the input string.
- pattern - the matched string.

Return value:

- the first occurrence offset on success, otherwise *nil* returned.

Example:

```
str = Import('str');
sys = Import('sys');

sys.print(str.strstr('this is a test', 'is'));
```

Output:

```
2
```



##### strncmp

String comparison of the first n characters.

```
str.strncmp(s1, s2, n);
```

Input:

- s1 - the first string.
- s2 - the second string
- n - number of characters to use in the comparison.

Return value:

- *true* returned on success, otherwise *false* returned.

Example:

```
str = Import('str');
sys = Import('sys');

sys.print(str.strncmp('this is a test', 'is', 2));
```

Output:

```
false
```



##### strcmp

String comparison.

```
str.strcmp(s1, s2);
```

Input:

- s1 - the first string.
- s2 - the second string.

Return value:

- *true* returned on success, otherwise *false* returned.

Example:

```
str = Import('str');
sys = Import('sys');

sys.print(str.strcmp('drink', 'drink'));
```

Output:

```
true
```



##### strseqcmp

String comparison.

```
str.strseqcmp(s1, s2);
```

Input:

- s1 - the first string.
- s2 - the second string.

Return value:

- an integer:
  - 1 - *s1* > *s2*
  - -1 - *s1* < *s2*
  - 0 - *s1* = *s2*

Example:

```
str = Import('str');
sys = Import('sys');

sys.print(str.strseqcmp('drinking', 'drink'));
```

Output:

```
1
```



##### reg_equal

Match string by a regular expression.

```
str.reg_equal(exp, text);
```

Input:

- exp - the input regular expression.
- text - the input text string.

Return value:

- *true* if completely matched, otherwise *false* returned.

Example:

```
str = Import('str');
sys = Import('sys');

sys.print(str.reg_equal('.*', 'test'));
sys.print(str.reg_equal('.*ed', 'test'));
```

Output:

```
true
false
```



##### reg_match

Match string by a regular expression and get matched string pieces.

```
str.reg_match(exp, text);
```

Input:

- exp - the input regular expression.
- text - the input text string.

Return value:

- an array that if matched anything, otherwise *false* returned.

Example:

```
str = Import('str');
sys = Import('sys');

sys.print(str.reg_match('((this )*i(s)).*', 'this is a reg exp test.'));
```

Output:

```
[this , s, this is, this is a reg exp test., ]
```



##### replace

Replace all occurrences of the search string with the replacement string.

```
str.replace(&dict, &str);
```

Input:

- `dict` - the dictionary contains search strings and replacement strings.
- `str` - the content string.

Return value:

- a string with the replaced values.

Example:

```
str = Import('str');
sys = Import('sys');

sys.print(str.replace(['aaa':'123', 'bbb': 'Hello'], 'aaabbbcccaaa'));
```

Output:

```
123Helloccc123
```



##### trim

Strip whitespace (or other characters) from the beginning and end of a string.

```
str.trim(s, mask);
```

Input:

- `s` - the string that will be trimmed.
- `mask` -  Optionally, the stripped characters can also be specified using the `mask` parameter. Simply list all characters that you want to be stripped.

Return value:

- the trimmed string.

Example:

```
str = Import('str');
sys = Import('sys');

sys.print(str.trim(" hello \t\n"));
```

Output:

```
hello
```



##### upper

Make character uppercase.

```
str.upper(s);
```

Input:

- `s` - the original string.

Return value:

- the string with uppercase.

Example:

```
str = Import('str');
sys = Import('sys');

sys.print(str.upper('hello'));
```

Output:

```
HELLO
```



##### lower

Make character lowercase.

```
str.lower(s);
```

Input:

- `s` - the original string.

Return value:

- the string with lowercase.

Example:

```
str = Import('str');
sys = Import('sys');

str.print(sys.lower('HELLO'));
```

Output:

```
hello
```



##### join

Join all elements in an array into a string, using a string as separator.

```
str.join(glue, arr);
```

Input:

- `glue` - the separator string.
- `arr` - the input array.

Return value:

- returns a string containing a string representation of all the array elements, with the separator string between each element. **The order of elements does not depend on the index, but the order in which they are appended to the array**.

Example:

```
str = Import('str');
sys = Import('sys');

sys.print(str.join(',', [1, [1,2], 'asd']));
```

Output:

```
1,Array,asd
```



##### capitalize

Capitalizes the first letter of a string.

```
str.capitalize(s);
```

Input:

- `s` - the input string.

Return value:

- returns a string with the first character capitalized.

Example:

```
str = Import('str');
sys = Import('sys');

sys.print(str.capitalize('abc'));
```

Output:

```
Abc
```

