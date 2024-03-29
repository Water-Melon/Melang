### System



### Import

```
sys = Import('sys');
```



##### print

Print argument.

```
sys.print(var);
```

Input:

- `var` - any type can be accepted.
  - if `var` is an array, elements in array will be printed.
  - if `var` is a function or object, only a string `function` or `object` will be printed.
  - otherwise `var`'s value will be printed.

Return value:

- Always `true`.



##### dump

Output detail of the given argument.

This is a built-in function, not implemented in dynamic library.

```
Dump(var);
```

Input:

- `var` - any type can be accepted.
  - array - output all array elements and array keys and more details.
  - object - output all properties' detail.
  - other type - output details of `var`.

Return value:

- `nil`

Example

```
sys = Import('sys');

a = [1, 2, 3];
sys.print(a);
Dump(a);
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



##### size

Get array length.

```
sys.size(&array);
```

Input:

- `array` - an array or dict.

Return value:

- the length of `array`.

Example:

```
sys = Import('sys');

sys.print(sys.size([1, 2, 3]));
```

Output:

```
3
```



##### is_int

Find whether the type of a variable is integer.

```
sys.is_int(var);
```

Input:

- `var` - a variable.

Return value:

- `true` if value is an integer, otherwise `false` returned.

Example:

```
sys = Import('sys');

sys.print(sys.is_int(1));
sys.print(sys.is_int('1'));
```

Output:

```
true
false
```



##### is_real

Find whether the type of a variable is real.

```
sys.is_real(var);
```

Input:

- `var` - a variable.

Return value:

- `true` if value is a real number, otherwise `false` returned.

Example:

```
sys = Import('sys');

sys.print(sys.is_real(1));
sys.print(sys.is_real(1.0));
```

Output:

```
false
true
```



##### is_str

Find whether the type of a variable is string.

```
sys.is_str(var);
```

Input:

- `var` - a variable.

Return value:

- `true` if value is a string, otherwise `false` returned.

Example:

```
sys.print(sys.is_str('123'));
```

Output:

```
true
```



##### is_nil

Find whether the type of a variable is nil.

```
sys.is_nil(var);
```

Input:

- `var` - a variable.

Return value:

- `true` if value is `nil`, otherwise `false` returned.

Example:

```
sys.print(sys.is_nil(0));
sys.print(sys.is_nil(nil));
```

Output:

```
false
true
```



##### is_bool

Find whether the type of a variable is bool.

```
sys.is_bool(var);
```

Input:

- `var` - a variable.

Return value:

- `true` if value is bool, otherwise `false` returned.

Example:

```
sys.print(sys.is_bool(0));
sys.print(sys.is_bool(false));
```

Output:

```
false
true
```



##### is_obj

Find whether the type of a variable is object.

```
is_obj(var);
```

Input:

- `var` - a variable.

Return value:

- `true` if value is an object, otherwise `false` returned.

Example:

```
sys = Import('sys');

set {}
o = $set;
sys.print(sys.is_obj(o));
```

Output:

```
true
```



##### is_func

Find whether the type of a variable is function.

```
sys.is_func(var);
```

Input:

- `var` - a variable.

Return value:

- `true` if value is a function, otherwise `false` returned.

Example:

```
sys = Import('sys');

@foo() {}
sys.print(sys.is_func(foo));
```

Output:

```
true
```



##### is_array

Find whether the type of a variable is array.

```
sys.is_array(var);
```

Input:

- `var` - a variable.

Return value:

- `true` if value is an array, otherwise `false` returned.

Example:

```
sys = Import('sys');

sys.print(sys.is_array([1, 2]));
sys.print(sys.is_array(['key1': 1, 'key2':2]));
```

Output:

```
true
true
```



##### int

Convert the variable to an integer value.

```
sys.int(var);
```

Input:

- `var` - a variable whose type should be: `int`, `real`, `string`.

Return value:

- an integer.

Example:

```
sys = Import('sys');

sys.print(sys.int(1.2));
sys.print(sys.int('1'));
```

Output:

```
1
1
```



##### bool

Convert the variable to a boolean value.

```
sys.bool(var);
```

Input:

- `var` - a variable.

Return value:

- a bool value.

Example:

```
sys = Import('sys');

sys.print(sys.bool(1.2));
sys.print(sys.bool(0));
sys.print(sys.bool(nil));
sys.print(sys.bool([]));
sys.print(sys.bool(''));
```

Output:

```
true
false
false
false
false
```



##### real

Convert the variable to a real number.

```
sys.real(var);
```

Input:

- `var` - a variable whose type should be: `int`, `real`, `string`.

Return value:

- a real number.

Example:

```
sys = Import('sys');

sys.print(sys.real(1));
sys.print(sys.real('1.2'));
```

Output:

```
1.000000
1.200000
```



##### str

Convert the variable to a string.

```
sys.str(var);
```

Input:

- `var` - a variable whose type should be: `int`, `bool`, `real`, `string`.

Return value:

- a string.

Example:

```
sys = Import('sys');

Dump(sys.str(1));
Dump(sys.str(1.2));
```

Output:

```
  var <Var>    Refer  Alias name: var  valueRef: 1, udata <0x0>, func <0x0>, notModify: false
    <STRING> '1'
  var <Var>    Refer  Alias name: var  valueRef: 1, udata <0x0>, func <0x0>, notModify: false
    <STRING> '1.200000'
```



##### obj

Convert the array or object to an object.

```
sys.obj(var);
```

Input:

- `var` - a variable whose type should be: `object`, `array`.

Return value:

- an object.

Example:

```
Dump(sys.obj([1, 2]));
Dump(sys.obj(['name': 'Tom', 'age': 18]));
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



##### array

Convert the array or object to an array.

```
sys.array(var);
```

Input:

- `var` - a variable.

Return value:

- an array.

Example:

```
sys = Import('sys');

Human {
  name;
  age;
}
o = $Human;
o.name = 'Tom';
o.age = 18;
arr = sys.array(o);
sys.print(arr);
sys.print(arr['name']);
```

Output:

```
[Tom, 18, ]
Tom
```



##### keys

Return all the keys or a subset of the keys of an array.

```
sys.keys(&array);
```

Input:

- `array` - the input array.

Return value:

- an array  of all non-integer keys in `array`.

Example:

```
sys = Import('sys');

a = ['name': 'Tom', 'age': 18];
sys.print(sys.keys(a));
```

Output:

```
[name, age, ]
```



##### merge

Merge two arrays

```
sys.merge(array1, array2);
```

Input:

- `array1` - the first array;
- `array2` - the second array.

Return value:

- a merged array.

Example:

```
sys = Import('sys');

a = ['name': 'Tom', 'age': 18];
b = ['name': 'Sam', 'age': 19];
c = [1, 2, 3];
d = [2, 3, 4];
sys.print(sys.merge(a, b));
sys.print(sys.merge(c, d));
ret = sys.merge(b, c);
sys.print(ret);
sys.print(ret['name']);
```

Output:

```
[Sam, 19, ]
[1, 2, 3, 2, 3, 4, ]
[Sam, 19, 1, 2, 3, ]
Sam
```



##### has

Checks whether the symbol string `thing` in `owner`.

```
sys.has(owner, thing);
```

Input:

There are threee types of `owner`:

- `nil`: `thing` will be searched in local and global symbol tables.

- array: `thing` will be treated as a array key searched in array `owner`.

- object: `thing` will be treated as a property searched in object `owner`.

The `thing` is an array key or a string name.

Return value:

There are values will be returned:

- `true`: if array key `thing` will be found in array `owner`.

- `'variable'`: if `thing` is a regular variable or an object variable.

- `'function'`: if `thing` is a regular function.

- `'method'`: if `thing` is an object method.

- `false`: found nothing.

Example:

```
sys = Import('sys');

a = ['name': 'Tom', 'age': 18];
sys.print(sys.has(a, 'name'));
```

Output:

```
true
```



##### type

Get the type of input argument.

```
sys.type(var);
```

Input:

- `var` - the input argument. It can be any types but Set.

Return value:

- type of `var`. If `var` is an object, Set name will be returned.

Example:

```
sys = Import('sys');

a = ['name': 'Tom', 'age': 18];
SetA {}
@foo () {}

sys.print(sys.type(1));
sys.print(sys.type(1.2));
sys.print(sys.type('abc'));
sys.print(sys.type([1, 2]));
sys.print(sys.type($SetA));
sys.print(sys.type(foo));
sys.print(sys.type(sys.obj(a)));
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



##### getter

Get property value from an object.

```
sys.getter(&obj, prop);
```

Input:

- `obj` - the input object.
- `prop` - the property name string. ***prop* must exist in *obj*.**

Return value:

- The value of `prop` in `obj`.

Example:

```
sys = Import('sys');

Human {
  name;
}
o = $Human;
o.name = 'Tom';
sys.print(sys.getter(o, 'name'));
```

Output:

```
Tom
```



##### setter

Set a property with its value in an object.

```
sys.setter(&obj, prop, &val);
```

Input:

- `obj` - the input object.
- `prop` - the property name string.
- `val` - the property value. This is an optional argument, `nil` as default.

Return value:

- the property value.

Example:

```
sys = Import('sys');

Human {
  name;
}
o = $Human;
o.name = 'Tom';
sys.print(sys.setter(o, 'age', 18));
sys.print(sys.setter(o, 'age'));
```

Output:

```
18
18
```



##### mkdir

Create a directory.

```
sys.mkdir(path, mode);
```

Input:

- `path` - a directory path in file system.
- `mode` - an optional argument. It's an integer indicating directory priorities, such as `0755`.

Return value:

- `true` on success, otherwise `false`.

Example:

```
sys = Import('sys');

sys.mkdir('/tmp/aaa');
```

```shell
$ ls -d /tmp/aaa
```

Output:

```
/tmp/aaa
```



##### remove

Remove directory from file system.

```
sys.remove(path);
```

Input:

- `path` - directory path.

Return value:

- `true` on success, otherwise `false`.

Example:

```
sys = Import('sys');

sys.print(sys.remove('/tmp/aaa'));
```

Output:

```
true
```



##### exist

Check if the file or directory exists.

```
sys.exist(path);
```

Input:

- `path` - file or directory path.

Return value:

- `true` if exists, otherwise `false`.

Example:

```
sys = Import('sys');

sys.print(sys.exist('/tmp'));
```

Output:

```
true
```



##### lsdir

List all files and directories under the specified path.

```
sys.lsdir(path);
```

Input:

- `path` - file or directory path.

Return value:

- an array contained all files and directories under the `path`.

Example:

```
sys = Import('sys');

sys.print(sys.path('/'));
```

Output:

```
[tmp, run, etc, lib64, home, sbin, proc, sys, ., usr, lost+found, root, boot, media, .., dev, bin, opt, lib, mnt, var, srv, ]
```



##### isdir

Check the given path is directory or not.

```
sys.isdir(path);
```

Input:

- `path` - file or directory path.

Return value:

- `true` if is directory, otherwise `false`.

Example:

```
sys = Import('sys');

sys.print(sys.exist('/'));
```

Output:

```
true
```



##### time

Get the current time seconds.

```
sys.time();
```

Input: *none*

Return value:

- an integer that is the current time seconds.

Example:

```
sys.print(sys.time());
```

Output:

```
1628567103
```



##### utctime

Return a UTC time string.

```
sys.utctime(tm);
```

Input:

- `tm` - an integer timestamp that might be generated by `sys.time`.

Return value:

- a UTC time string.

Example:

```
sys.print(sys.utctime(sys.time()));
```

Output:

```
10/25/2023 02:56:32 UTC
```



##### cron

Parse cron format expression and get the next allowed timestamp.

```
sys.cron(exp, timestamp);
```

Input:

- `exp` - a string cron format expression.
- `timestamp` - an integer timestamp which can be generated by function `mln_time`.

Return value:

- the next allowed integer timestamp in seconds.

Example:

```
sys = Import('sys');

tm = sys.time();
sys.print(tm);
sys.print(sys.cron('* * * * *', tm));
```

Output:

```
1628676762
1628676822
```



##### diff

Compute the difference of arrays.

```
sys.diff(&array1, &array2);
```

Input:

- `array1` - the array to compare from.
- `array2` - the array to compare against

Return value:

- Returns an array containing all the entries from `array1` that are not present in `array2`. Keys in the `array1` are preserved.

Example:

```
sys = Import('sys');

sys.print(sys.diff([1,2,3,4,5], [2,4,5]));
```

Output:

```
[1, 3, ]
```



##### key_diff

Compute the difference of arrays by keys.

```
sys.key_diff(&array1, &array2);
```

Input:

- `array1` - the array to compare from.
- `array2` - the array to compare against

Return value:

- Returns an array containing all the entries whose key is from `array1` but not in `array2`. Keys in the `array1` are preserved.

Example:

```
sys = Import('sys');

sys.print(sys.key_diff(['aaa':1,2,'ccc':3,'ddd':4, 5], ['aaa':1,2,3]););
```

Output:

```
[3, 4, ]
```



##### exec

Execute shell command in Melang.

```
sys.exec(cmd, bufsize, pid, uid, gid, qname);
```

Description: Execute shell command or list all running commands.

Input:

- `cmd` - shell command string. If `cmd` is `nil`, this function will return a list of running commands.
- `bufsize` - the limit size of the command output. if `<0` or omitted, it means no limitation.
- `pid` - is an output argument. After this function returned, the child process id will be given in this variable.
- `user` - the user of the child process. It is optional.
- `group` - the group of the child process. It is optional, and if it is omitted and `user` is given, then the group of the child process will be identical with `user`.
- `qname` - the message queue name that the output of new process can be retrieved from this message queue by function `mq.recv` which is in the message queue module.

Return value:

- On Windows, this function only return `nil`.
- On UNIX-like system, `false` on failure, otherwise is the command output.
- On both kinds of systems, return a list of running commands if `cmd` is `nil`.

Example:

```
sys = Import('sys');

sys.exec('ls /');
```

Output:

```
bin
boot
dev
etc
...
```



##### import

Import dynamic library extension into current scope. Dynamic library may contain functions, collections, variables, etc.

This is a built-in function.

```
Import(name);
```

Input:

- `name` - Dynamic library name (not include `.so` or `.dll`). `name` can be a relative path, an absolute path or just a name.
           But if it is just a name, the library will be found in the following path:
           1. current directory
           2. Directories included in environment `MELANG_DYNAMIC_PATH`
           3. `/usr/local/lib/melang_dynamic` (on UNIX) or `$HOME/lib/melang_dynamic` (on Windows)

Return value:

- `nil`

Example:

```
Import('test');
```

```c
//test.c -> test.so or test.dll in current directory
#include <stdio.h>
#include "mln_lang.h"
int init(mln_lang_ctx_t *ctx)
{
    printf("%s\n", __FUNCTION__);
    return 0;
}
```

Output:

```
init
```



##### msleep

Execute shell command in Melang.

```
sys.msleep(msec);
```

Input:

- `msec` - timeout milliseconds.

Return value:

- `nil`

Example:

```
sys = Import('sys');

sys.msleep(1000); //1 second
```



##### path

For the path given by the parameter, escape the starting @ symbol to the current directory path of the script.

```
sys.path(path);
```

Input:

- `path` - A path string that starts with @.

Return value:

- The path after translation.

Example:

```
// xxx/test.m

sys = Import('sys');

sys.path('@/a.m');
```

Run script
```
melang xxx/test.m
```

The output is
```
xxx/a.m
```

