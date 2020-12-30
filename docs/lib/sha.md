### SHA



##### mln_sha1

```
@mln_sha1(data);
```

Input:

- data - a string or an opened MFile object.

Return value:

- a digest string of *data*.

Error:

- Invalid argument will throw an error.



##### mln_sha256

```
@mln_sha1(data);
```

Input:

- data - a string or an opened MFile object.

Return value:

- a digest string of *data*.

Error:

- Invalid argument will throw an error.



##### Example

```
@mln_print(@mln_sha1('sha test'));
@mln_print(@mln_sha256('sha test'));
```

The output is:

```
d8d4c749ea2cbdc7c8469367d1601509d4a213ef
ed8da5086c551c8d3f3e637fecff432639e3190a24605f363b91444f82fee554
```

