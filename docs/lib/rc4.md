### RC4



### Import

```
rc = Import('rc');
```



##### rc4

```
rc.rc4(data, key);
```

Input:

- `data` - a string value which is a text or ciphertext.
- `key` - a string value.

Return value:

- a encrypted or decrypted string.

Error:

- Invalid argument will throw an error.

Example:

```
rc = Import('rc');
sys = Import('sys');

cipher = rc.rc4('This is a rc4 test', 'a key');
sys.print(cipher);
sys.print(rc.rc4(cipher, 'a key'));
```

The output is:

```
oO?L?RQ??????(?
This is a rc4 test
```

