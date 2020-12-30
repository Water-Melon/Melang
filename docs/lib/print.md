### Print



##### mln_print

Print argument.

```
@mln_print(var);
```

Input:

- var - any type can be accepted.
  - if *var* is array, array values will be printed.
  - if *var* is function or object, only a string *function* or *object* will be printed.
  - otherwise *var*'s value will be printed.

Return value:

- Always *true*.



##### mln_dump

Output detail of the given argument.

```
@mln_dump(var);
```

Input:

- var - any type can be accepted.
  - array - output all array values and array keys and more other details.
  - object - output all properties detail information.
  - other type - output detail information of *var*.

Return value:



##### Example

```
a = [1, 2, 3];
@mln_print(a);
@mln_dump(a);
```

The output is:

```
[1, 2, 3, ]
  var <Var>    Refer  Alias name: var  valueRef: 2, udata <0x0>, func <0x0>, notModify: false
    <ARRAY>
    ALL ELEMENTS:
      Index: 0
      Value:
        Normal  valueRef: 1, udata <0x0>, func <0x0>, notModify: false
        <INT> 1
      Index: 1
      Value:
        Normal  valueRef: 1, udata <0x0>, func <0x0>, notModify: false
        <INT> 2
      Index: 2
      Value:
        Normal  valueRef: 1, udata <0x0>, func <0x0>, notModify: false
        <INT> 3
    KEY ELEMENTS:
    Refs: 2
```

