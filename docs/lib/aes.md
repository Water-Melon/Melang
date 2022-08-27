### AES

This is a library function for AES encryption and decryption.



### Import

```
aes = import('aes');
```



### Functions

#### aes

```
aes.aes(data, key, bits, op);
```

Input:

- data -- a string value and its length can be divided by 16.
- key --  a string value for *data* encryption and decryption.
- bits -- an integer value of the *key* bit length. *Key*'s' bit-length must be equal to or greater than *bits*. It supports these value: *128*, *192*, *256*.

- op -- a string value to indicate function to encrypt or decrypt. It only has two values: *encode* and *decode*.

Return value:

- The encrypted cipher string.

Error:

- If encounter an invalid argument, interpreter will throw an error and stop executing.

Example:

```
aes = import('aes');
sys = import('sys');
str = import('str');

text = 'This is an aes test, note length';
sys.print('text length: '+str.strlen(text));
key = 'this is a secret';
cipher = aes.aes(text, key, 128, 'encode');
sys.print(cipher);
text = aes.aes(cipher, key, 128, 'decode');
sys.print(text);
```

The output is:

```
text length: 32
l???<??J? ?H?D???j?8?!??B#?8
This is an aes test, note length
```

