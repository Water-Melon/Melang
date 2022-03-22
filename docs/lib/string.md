### String



##### mln_bin2hex

Binary bytes string to hex string.

```
mln_bin2hex(bin);
```

Input:

- bin - a binary bytes string.

Return value:

- a hex string.

Example:

```
mln_print(mln_bin2hex(mln_rc4('HI', 'key')));
```

Output:

```
4325
```



##### mln_hex2bin

Hex string to binary bytes.

```
mln_hex2bin(hex);
```

Input:

- hex - a hex string.

Return value:

- a binary bytes string.

Example:

```
mln_print(mln_hex2bin('4849'));
```

Output:

```
HI
```



##### mln_bin2int

Binary bytes to an integer.

```
mln_bin2int(bin);
```

Input:

- bin - a binary bytes string.

Return value:

- an integer.

Example:

```
mln_print(mln_bin2int('HI'));
```

Output:

```
18505
```



##### mln_int2bin

Integer to binary bytes.

```
mln_int2bin(i);
```

Input:

- i - an integer.

Return value:

- a binary bytes string.

Example:

```
mln_print(mln_int2bin(18505));
```

Output:

```
HI
```



##### mln_bin2real

Binary bytes to real number.

```
mln_bin2real(bin);
```

Input:

- a binary bytes string.

Return value:

- a real number.

Example:

```
mln_print(mln_bin2real('HI'));
```

Output:

```
1070435769529469910793714477087121352287059968.000000
```



##### mln_real2bin

Real number to binary bytes.

```
mln_real2bin(r);
```

Input:

- a real number.

Return value:

- a binary bytes.

Example:

```
mln_print(mln_real2bin(1070435769529469910793714477087121352287059968.000000));
```

Output:

```
HI
```



##### mln_b2s

Copy binary bytes into a string.

```
mln_b2s(bin);
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
mln_print(mln_b2s(100));
```

Output:

```
d
```



##### mln_s2b

String to a given type value.

```
mln_s2b(s, type);
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
mln_print(mln_s2b('d', 'int'));
```

Output:

```
100
```



##### mln_strlen

Calculate string length.

```
mln_strlen(s);
```

Input:

- a string value.

Return value:

- an integer string length.

Example:

```
mln_print(mln_strlen('this is a test'));
```

Output:

```
14
```



##### mln_split

Return part of a string.

```
mln_split(s, offset, len);
```

Input:

- s - a string value.
- offset - an integer offset.
- len - is optional. It should be a positive number or zero. If not set, *len* will be set as *string length - offset*.

Return value:

- The splitted string.

Example:

```
mln_print(mln_split('this is a test', -10));
```

Output:

```
 is a test
```



##### mln_slice

Split a string by a set of characters.

```
mln_slice(s, seps);
```

Input:

- s - the input string.
- seps - a set of characters, a string.

Return value:

- Returns an array of strings created by splitting the *s* parameter on boundaries formed by the *seps*.

Example:

```
mln_print(mln_slice('this is a test', ' s'));
```

Output:

```
[thi, i, a, te, t, ]
```



##### mln_kmp

Find the first occurrence of a string by KMP algorithm.

```
mln_kmp(s, pattern);
```

Input:

- s - the input string.
- pattern - the matched string.

Return value:

- the first occurrence offset on success, otherwise *nil* returned.

Example:

```
mln_print(mln_kmp('this is a test', 'is'));
```

Output:

```
2
```



##### mln_strstr

Find the first occurrence of a string

```
mln_strstr(s, pattern);
```

Input:

- s - the input string.
- pattern - the matched string.

Return value:

- the first occurrence offset on success, otherwise *nil* returned.

Example:

```
mln_print(mln_strstr('this is a test', 'is'));
```

Output:

```
2
```



##### mln_strncmp

String comparison of the first n characters.

```
mln_strncmp(s1, s2, n);
```

Input:

- s1 - the first string.
- s2 - the second string
- n - number of characters to use in the comparison.

Return value:

- *true* returned on success, otherwise *false* returned.

Example:

```
mln_print(mln_strncmp('this is a test', 'is', 2));
```

Output:

```
false
```



##### mln_strcmp

String comparison.

```
mln_strcmp(s1, s2);
```

Input:

- s1 - the first string.
- s2 - the second string.

Return value:

- *true* returned on success, otherwise *false* returned.

Example:

```
mln_print(mln_strcmp('drink', 'drink'));
```

Output:

```
true
```



##### mln_strseqcmp

String comparison.

```
mln_strseqcmp(s1, s2);
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
mln_print(mln_strseqcmp('drinking', 'drink'));
```

Output:

```
1
```



##### mln_reg_equal

Match string by a regular expression.

```
mln_reg_equal(exp, text);
```

Input:

- exp - the input regular expression.
- text - the input text string.

Return value:

- *true* if completely matched, otherwise *false* returned.

Example:

```
mln_print(mln_reg_equal('.*', 'test'));
mln_print(mln_reg_equal('.*ed', 'test'));
```

Output:

```
true
false
```



##### mln_reg_match

Match string by a regular expression and get matched string pieces.

```
mln_reg_match(exp, text);
```

Input:

- exp - the input regular expression.
- text - the input text string.

Return value:

- an array that if matched anything, otherwise *false* returned.

Example:

```
mln_print(mln_reg_match('((this )*i(s)).*', 'this is a reg exp test.'));
```

Output:

```
[this , s, this is, this is a reg exp test., ]
```



##### mln_replace

Replace all occurrences of the search string with the replacement string.

```
mln_replace(&dict, &str);
```

Input:

- `dict` - the dictionary contains search strings and replacement strings.
- `str` - the content string.

Return value:

- a string with the replaced values.

Example:

```
mln_print(mln_replace(['aaa':'123', 'bbb': 'Hello'], 'aaabbbcccaaa'));
```

Output:

```
123Helloccc123
```

