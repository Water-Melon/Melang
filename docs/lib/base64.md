### Base64



### Import

```
b = Import('base64');
```



##### base64

```
b.base64(data, op);
```

Input:

- `data` - string type value.
- `op` - a string value to indicate function to encode or decode. It only has two values: `encode` and `decode`.

Return value:

- The encoded or decoded string.

Error:

- If argument is invalid, an error will be occurred.

Example:

```
b = Import('base64');
sys = Import('sys');

text = 'base64 test';
encoded = b.base64(text, 'encode');
sys.print(encoded);
sys.print(b.base64(encoded, 'decode'));
```

The output is:

```
YmFzZTY0IHRlc3Q=
base64 test
```

