### SHA



### Import

```
sha = Import('sha');
```



##### sha1

```
sha.sha1(data);
```

Input:

- `data` - a string or an opened File object.

Return value:

- a digest string of `data`.

Error:

- Invalid argument will throw an error.



##### sha256

```
sha.sha1(data);
```

Input:

- `data` - a string or an opened File object.

Return value:

- a digest string of `data`.

Error:

- Invalid argument will throw an error.



##### Example

```
sha = Import('sha');
sys = Import('sys');

sys.print(sha.sha1('sha test'));
sys.print(sha.sha256('sha test'));
```

The output is:

```
d8d4c749ea2cbdc7c8469367d1601509d4a213ef
ed8da5086c551c8d3f3e637fecff432639e3190a24605f363b91444f82fee554
```

