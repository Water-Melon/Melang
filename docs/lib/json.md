### JSON



##### mln_json_encode

```
@mln_json_encode(array);
```

Input:

- array - must be an array (common array or dict).

Return value:

- a encoded JSON string on success, otherwise *false* returned.

Error:

- An error will be occurred if encounter Invalid argument.



##### mln_json_decode

```
@mln_json_decode(s);
```

Input:

- s - must be a string.

Return value:

- a decoded JSON array on success, otherwise *false* returned.

Error:

- An error will be occurred if encounter Invalid argument.



##### Example

```
a = ['name': 'Tom', 'age': 18, 'gender': 'male'];
json = @mln_json_encode(a);
@mln_print(json);
@mln_print(@mln_json_decode(json));
```

The output is:

```
{"age":18.000000,"name":"Tom","gender":"male"}
[18.000000, Tom, male, ]
```

