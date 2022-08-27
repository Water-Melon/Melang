### MD5



### Import

```
m = import('md5');
```



##### md5

This function is used to calculate MD5 digest.

```
m.md5(data);
```

Input:

- data - a string or an opened File object.

Return value:

- The digest of string or file content one success. If read file failed, *false* will be returned.

Error:

- Invalid argument will throw an error.

Example:

```
m = import('md5');
sys = import('sys');

sys.print(m.md5('md5 test'));
```

The output is:

```
2e5f9458bcd27e3c2b5908af0b91551a
```

