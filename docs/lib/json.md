### JSON



### Import

```
j = Import('json');
```



##### encode

```
j.encode(array);
```

Input:

- `array` - must be an array (regular array or dict).

Return value:

- a encoded JSON string on success, otherwise `false` returned.

Error:

- An error will be occurred if encounter invalid argument.



##### decode

```
j.decode(s);
```

Input:

- `s` - must be a string.

Return value:

- a decoded JSON array on success, otherwise `false` returned.

Error:

- An error will be occurred if encounter Invalid argument.



##### Example

```
j = Import('json');
sys = Import('sys');

a = ['name': 'Tom', 'age': 18, 'gender': 'male'];
json = j.encode(a);
sys.print(json);
sys.print(j.decode(json));
```

The output is:

```
{"age":18.000000,"name":"Tom","gender":"male"}
[18.000000, Tom, male, ]
```

