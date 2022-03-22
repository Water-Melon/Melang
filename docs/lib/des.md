### DES

This document includes DES and 3DES functions.



##### mln_des

```
mln_des(data, key, op);
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
text = 'DES test'; //length is 8
cipher = mln_des(text, 100, 'encode');
mln_print(cipher);
mln_print(mln_des(cipher, 100, 'decode'));
```

The output is:

```
4xF???Q
DES test
```



##### mln_3des

```
mln_3des(data, key1, key2, op);
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
text = 'DES test'; //length is 8
cipher = mln_3des(text, 100, 99, 'encode');
mln_print(cipher);
mln_print(mln_3des(cipher, 100, 99, 'decode'));
```

The output is:

```
t?M??*?
DES test
```

