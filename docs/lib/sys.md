### System



##### mln_size

Get array length.

```
@mln_size(array);
```

Input:

- array - an array or dict.

Return value:

- the length of *array*.

Example:

```
@mln_print(@mln_size([1, 2, 3]));
```

Output:

```
3
```



##### mln_isInt

Find whether the type of a variable is integer.

```
@mln_isInt(var);
```

Input:

- var - a variable.

Return value:

- *true* if value is an integer, otherwise *false* returned.

Example:

```
@mln_print(@mln_isInt(1));
@mln_print(@mln_isInt('1'));
```

Output:

```
true
false
```



##### mln_isReal

Find whether the type of a variable is real.

```
@mln_isReal(var); 
```

Input:

- var - a variable.

Return value:

- *true* if value is a real number, otherwise *false* returned.

Example:

```
@mln_print(@mln_isReal(1));
@mln_print(@mln_isReal(1.0));
```

Output:

```
false
true
```



##### mln_isStr

Find whether the type of a variable is string.

```
@mln_isStr(var); 
```

Input:

- var - a variable.

Return value:

- *true* if value is a string, otherwise *false* returned.

Example:

```
@mln_print(@mln_isStr('123'));
```

Output:

```
true
```



##### mln_isNil

Find whether the type of a variable is nil.

```
@mln_isNil(var);
```

Input:

- var - a variable.

Return value:

- *true* if value is *nil*, otherwise *false* returned.

Example:

```
@mln_print(@mln_isNil(0));
@mln_print(@mln_isNil(nil));
```

Output:

```
false
true
```



##### mln_isBool

Find whether the type of a variable is bool.

```
@mln_isBool(var);
```

Input:

- var - a variable.

Return value:

- *true* if value is bool, otherwise *false* returned.

Example:

```
@mln_print(@mln_isBool(0));
@mln_print(@mln_isBool(false));
```

Output:

```
false
true
```



##### mln_isObj

Find whether the type of a variable is object.

```
@mln_isObj(var);
```

Input:

- var - a variable.

Return value:

- *true* if value is an object, otherwise *false* returned.

Example:

```
set {}
o = $set;
@mln_print(@mln_isObj(o));
```

Output:

```
true
```



##### mln_isFunc

Find whether the type of a variable is function.

```
@mln_isFunc(var); 
```

Input:

- var - a variable.

Return value:

- *true* if value is a function, otherwise *false* returned.

Example:

```
@foo() {}
@mln_print(@mln_isFunc(foo));
```

Output:

```
true
```



##### mln_isArray

Find whether the type of a variable is array.

```
@mln_isArray(var);
```

Input:

- var - a variable.

Return value:

- *true* if value is an array, otherwise *false* returned.

Example:

```
@mln_print(@mln_isArray([1, 2]));
@mln_print(@mln_isArray(['key1': 1, 'key2':2]));
```

Output:

```
true
true
```



##### mln_int

Get int value of a variable.

```
@mln_int(var);
```

Input:

- var - a variable whose type should be: *int, real, string*.

Return value:

- an integer.

Example:

```
@mln_print(@mln_int(1.2));
@mln_print(@mln_int('1'));
```

Output:

```
1
1
```



##### mln_bool

Get bool value of a variable.

```
@mln_bool(var);
```

Input:

- var - a variable.

Return value:

- a bool value.

Example:

```
@mln_print(@mln_bool(1.2));
@mln_print(@mln_bool(0));
@mln_print(@mln_bool(nil));
@mln_print(@mln_bool([]));
@mln_print(@mln_bool(''));
```

Output:

```
true
false
false
false
false
```



##### mln_real

Get real value of a variable.

```
@mln_real(var);
```

Input:

- var - a variable whose type should be: *int, real, string*.

Return value:

- a real number.

Example:

```
@mln_print(@mln_real(1));
@mln_print(@mln_real('1.2'));
```

Output:

```
1.000000
1.200000
```



##### mln_str

Get string value of a variable.

```
@mln_str(var);
```

Input:

- var - a variable whose type should be: *int, bool, real, string*.

Return value:

- a string.

Example:

```
@mln_dump(@mln_str(1));
@mln_dump(@mln_str(1.2));
```

Output:

```
  var <Var>    Refer  Alias name: var  valueRef: 1, udata <0x0>, func <0x0>, notModify: false
    <STRING> '1'
  var <Var>    Refer  Alias name: var  valueRef: 1, udata <0x0>, func <0x0>, notModify: false
    <STRING> '1.200000'
```



##### mln_obj

Get object value of a variable.

```
@mln_obj(var);
```

Input:

- var - a variable whose type should be: *object, array*.

Return value:

- an object.

Example:

```
@mln_dump(@mln_obj([1, 2]));
@mln_dump(@mln_obj(['name': 'Tom', 'age': 18]));
```

Output:

```
  var <Var>    Refer  Alias name: var  valueRef: 1, udata <0x0>, func <0x0>, notModify: false
    <OBJECT> In Set '<anonymous>'  setRef: <unknown> objRef: 2
  var <Var>    Refer  Alias name: var  valueRef: 1, udata <0x0>, func <0x0>, notModify: false
    <OBJECT> In Set '<anonymous>'  setRef: <unknown> objRef: 2
      Normal  Alias name: name  valueRef: 1, udata <0x0>, func <0x0>, notModify: false
      <STRING> 'Tom'
      Normal  Alias name: age  valueRef: 1, udata <0x0>, func <0x0>, notModify: false
      <INT> 18
```

As we can see in output, common array will not generate any properties in object.



##### mln_array

Get array value of a variable.

```
@mln_array(var);
```

Input:

- var - a variable.

Return value:

- an array.

Example:

```
Human {
  name;
  age;
}
o = $Human;
o.name = 'Tom';
o.age = 18;
arr = @mln_array(o);
@mln_print(arr);
@mln_print(arr['name']);
```

Output:

```
[Tom, 18, ]
Tom
```



##### mln_keys

Return all the keys or a subset of the keys of an array.

```
@mln_keys(array);
```

Input:

- the input array.

Return value:

- an array  of all the keys in *array*.

Example:

```
a = ['name': 'Tom', 'age': 18];
@mln_print(@mln_keys(a));
```

Output:

```
[name, age, ]
```



##### mln_merge

Merge two arrays

```
@mln_merge(array1, array2);
```

Input:

- array1 - the first array;
- array2 - the second array.

Return value:

- a merged array.

Example:

```
a = ['name': 'Tom', 'age': 18];
b = ['name': 'Sam', 'age': 19];
c = [1, 2, 3];
d = [2, 3, 4];
@mln_print(@mln_merge(a, b));
@mln_print(@mln_merge(c, d));
ret = @mln_merge(b, c);
@mln_print(ret);
@mln_print(ret['name']);
```

Output:

```
[Sam, 19, ]
[1, 2, 3, 2, 3, 4, ]
[Sam, 19, 1, 2, 3, ]
Sam
```



##### mln_has

Checks if a key or property exists in an array or object.

```
@mln_has(owner, thing);
```

Input:

- owner - an array or object.
- thing - a key or a property name.

Return value:

- *true* if exists, otherwise *false*.

Example:

```
a = ['name': 'Tom', 'age': 18];
@mln_print(@mln_has(a, 'name'));
```

Output:

```
true
```



##### mln_type

Get the type of input argument.

```
@mln_type(var);
```

Input:

- var - the input argument. It can be any types but Set.

Return value:

- type of *var*. If *var* is an object, Set name will be returned.

Example:

```
a = ['name': 'Tom', 'age': 18];
SetA {}
@foo () {}

@mln_print(@mln_type(1));
@mln_print(@mln_type(1.2));
@mln_print(@mln_type('abc'));
@mln_print(@mln_type([1, 2]));
@mln_print(@mln_type($SetA));
@mln_print(@mln_type(foo));
@mln_print(@mln_type(@mln_obj(a)));
```

Output:

```
int
real
string
array
SetA
function
object
```



##### mln_getProperty

Get property value from an object.

```
@mln_getProperty(&obj, prop);
```

Input:

- obj - the input object.
- prop - the property name string. ***prop* must exist in *obj*.**

Return value:

- The value of *prop* in *obj*.

Example:

```
Human {
  name;
}
o = $Human;
o.name = 'Tom';
@mln_print(@mln_getProperty(o, 'name'));
```

Output:

```
Tom
```



##### mln_setProperty

Set a property with its value in an object.

```
@mln_setProperty(&obj, prop, &val);
```

Input:

- obj - the input object.
- prop - the property name string.
- val - the property value. This is an optional argument, *nil* as default.

Return value:

- the property value.

Example:

```
Human {
  name;
}
o = $Human;
o.name = 'Tom';
@mln_print(@mln_setProperty(o, 'age', 18));
@mln_print(@mln_getProperty(o, 'age'));
```

Output:

```
18
18
```



##### mln_eval

Create a new coroutine and execute.

```
@mln_eval(val, data, in_string);
```

Input:

- val - a file path or a code string.
- data - the data that we want to deliver to the new coroutine.
- In_string - a optional argument. If is set and its value is *true*, *val* is a code string. Otherwise, *val* is a file path.

Return value:

- always *nil*.

Example: visit [Coroutine Section](https://water-melon.github.io/Melang/coroutine.html).



##### mln_mkdir

Create a directory.

```
@mln_mkdir(path, mode);
```

Input:

- path - a directory path in file system.
- mode - an optional argument. It's an integer indicating directory priorities, such as 0755.

Return value:

- *true* on success, otherwise *false*.

Example:

```
@mln_mkdir('/tmp/aaa');
```

```shell
$ ls -d /tmp/aaa
```

Output:

```
/tmp/aaa
```



##### mln_remove

Remove directory from file system.

```
@mln_remove(path);
```

Input:

- path - directory path.

Return value:

- *true* on success, otherwise *false*.

Example:

```
@mln_print(@mln_remove('/tmp/aaa'));
```

Output:

```
true
```



##### mln_exist

Check if the file or directory exists.

```
@mln_exist(path);
```

Input:

- path - file or directory path.

Return value:

- *true* if exists, otherwise *false*.

Example:

```
@mln_print(@mln_exist('/tmp'));
```

Output:

```
true
```

