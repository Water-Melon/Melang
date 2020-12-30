### RC4



##### mln_rc4

```
@mln_rc4(data, key);
```

Input:

- data - a string value which is a text or ciphertext.
- key - a string value.

Return value:

- a encrypted or decrypted string.

Error:

- Invalid argument will throw an error.

Example:

```
cipher = @mln_rc4('This is a rc4 test', 'a key');
@mln_print(cipher);
@mln_print(@mln_rc4(cipher, 'a key'));
```

The output is:

```
oO?L?RQ??????(?
This is a rc4 test
```

