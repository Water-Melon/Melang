### File



### import

```
f = Import('file');
```



File is implemented as a Set named File.

```
File {
  fd;
  errno;
  open(path, op, prio);
  lseek(offset, whence);
  read(nbytes);
  write(data);
  close();
  errmsg();
  size();
};
```



##### fd

fd is the file descriptor that is like an integer in C. But in Melang, this property is read-only. It will be set by method *open*. Usually, we don't have to access it.



##### errno

errno is an integer as in C. This number is an error number set by every file operation method in this Set.



##### open

Open a file in the specified mode.

Input:

- path - this is a file path string.

- op - a string that indicates the operation including *read*, *write* and *append*. Its value is composed of three parts:

  - r - read
  - w - write
  - a - append
  - \+ - not truncate

  e.g.

  - rw - read and write
  - ra - read from the end of file
  - aw - append to write

- prio - an integer indicates the file access authority, e.g. 0777. This argument only used on opening a nonexistent file. It is optional. If not given, 0644 will be set by default.

Return value:

- *true* will be returned on success, otherwise *false* returned and *errno* will be set.



##### lseek

Repositions the offset of the file descriptor to the argument *offset*, according to the directive *whence*.

Input:

- offset - an integer offset.
- whence - a string value including three values:
  - begin - from the beginning of the file.
  - current - from current position.
  - end - from the end of the file.

Return value:

- *true* will be returned on success, otherwise *false* returned and *errno* will be set.



##### read

Read *nbytes* from current position of the openned file.

Input:

- nbytes - an integer indicates how many bytes read in this time.

Return value:

- *nbytes* data in file will be returned on success, otherwise *false* returned and errno will be set.



##### write

Write *data* in the file.

Input:

- data - must be a string.

Return value:

- the number of bytes written in file will be returned on success, otherwise *false* returned and errno will be set.



##### close

Close file.

Input: None

Return value:

- Always be *nil*.



#### errmsg

Get error message.

Input: None

Return value:

- a string error message will be returned.



##### size

Get file size.

Input: None

Return value:

- an integer file bytes will be returned on success, otherwise *false* returned.



##### Example

Create a test file before Melang execution.

```shell
$ echo "hello" > tempfile
```

Execute Melang program:

```
sys = Import('sys');
F = Import('file');
f = $F; // or f = $File; both are the same.
if (f.open('tempfile', 'rw+') != false) { //read and write file and file content won't be ereased
  f.lseek(1, 'begin');
  f.write('hi all');
  f.lseek(0, 'begin');
  sys.print(f.read(f.size()));
} fi
```

The output is:

```
hi all
```

And the content in tempfile is:

```
hi all
```

