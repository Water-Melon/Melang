### DES

This document includes DES and 3DES functions.



### Import

```
d = Import('des');
```



##### des

```
d.des(data, key, op);
```

Input:

- data - string value and its length % 8 must be 0.
- key - must be an integer value.
- op - a string value to indicate function to encrypt or decrypt. It only has two values: *encode* and *decode*.

Return value:

- an encrypted or decrypted string.

Error:

- If encounter an invalid argument, interpreter will throw an error and stop executing.

Example:

```
d = Import('des');
sys = Import('sys');

text = 'DES test'; //length is 8
cipher = d.des(text, 100, 'encode');
sys.print(cipher);
sys.print(d.des(cipher, 100, 'decode'));
```

The output is:

```
4xF???Q
DES test
```



##### des3

```
des3(data, key1, key2, op);
```

Input:

- data - string value and its length % 8 must be 0.
- key1, key2 - must be an integer value.
- op - a string value to indicate function to encrypt or decrypt. It only has two values: *encode* and *decode*.

Return value:

- an encrypted or decrypted string.

Error:

- If encounter an invalid argument, interpreter will throw an error and stop executing.

Example:

```
d = Import('des');
sys = Import('sys');

text = 'DES test'; //length is 8
cipher = d.des3(text, 100, 99, 'encode');
sys.print(cipher);
sys.print(d.des3(cipher, 100, 99, 'decode'));
```

The output is:

```
t?M??*?
DES test
```

