### Preprocess

Like preprocess in C, Melang supports simple preprocess.



#### macro

```
#define NAME 'John' //define macro
#NAME //use macro
#undef NAME //undefine macro
```

*Name* is equivalent to 'John'. Melang just simply replace 'NAME' with 'John' in code.



#### condition

```
//example A
#if NAME
... //code A
#endif

//example B
#if NAME
... //code B
#else
... //code C
#endif
```

In example A, if *NAME* is defined, code A will be executed.

In example B, if NAME is defined, code B will be executed, otherwise execute code C.



#### include

Just like *include* in C, Melang also support it to load some script files into a file.

**Note: the included file must be quoted by double quotes.**

e.g.

```
#include "~/lib.mln"
// or load all files in a directory
#include "~/"
//files that are in sub-directories or prefixed by '.' will not be included
```

The path should follow these rules:

- if path is an absolute path, just try to load it.

- if path is not a absolute path, interpreter will search for file in the following order:

  1. if there is the file in current directory, just load it.
  2. if there is a environment variable named *MELANG_PATH* which points to some directories, interpreter will search for file in them. If interpreter cannot find file, an error would be occurred.
  3. if step 1 and step 2 are failed, interpreter will search for file in */usr/lib/melang/*.

  If failed, an error would be occurred.

Another example:

```
//in /xxx/yyy/a.m

#include "@/b.m"
```

`@` is a special char stands for base directory of current file (`a.m`). So its value is `/xxx/yyy'.

So `/xxx/yyy/b.m` will be included in `a.m`.
