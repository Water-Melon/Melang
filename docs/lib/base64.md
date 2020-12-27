### Base64



##### mln_base64

```
@mln_base64(data, op);
```

Input:

- data - string type value.
- op - a string value to indicate function to encode or decode. It only has two values: *encode* and *decode*.

Return value:

- The encoded or decoded string.

Error:

- If argument is invalid, an error will be occurred.

Example:

```
text = 'base64 test';
encoded = @mln_base64(text, 'encode');
@mln_print(encoded);
@mln_print(@mln_base64(encoded, 'decode'));
```

The output is:

```
YmFzZTY0IHRlc3Q=
base64 test
```

