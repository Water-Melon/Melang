### Matrix



### Import

```
m = Import('matrix');
```



##### mul

Matrix multiplication.

```
m.mul(array1, array2);
```

Input:

- *array1* and *array2* are dicts those format must be:

  ```
  ['row': rownum, 'col': colnum, 'data':[...]]
  ```

  e.g.

  ```
  ['row': 3, 'col': 2, 'data':[1,0,0,1,1,1]]
  ```

  Which means this is a matrix shown below:

  ```
  [
    [1, 0],
    [0, 1],
    [1, 1],
  ]
  ```

Return value:

- A dict indicates the matrix that is the result of two arguments multiplication.

Error:

- An error occurred if encounter an invalid argument.

Example:

```
m = Import('matrix');
sys = Import('sys');

array1 = ['row': 3, 'col': 2, 'data':[1,0,0,1,1,1]];
array2 = ['row': 2, 'col': 2, 'data':[1,2,3,4]];
res_array = m.mul(array1, array2);
sys.print(res_array['row']);
sys.print(res_array['col']);
sys.print(res_array['data']);
```

The output is:

```
3
2
[1.000000, 2.000000, 3.000000, 4.000000, 4.000000, 6.000000, ]
```



##### inv

Matrix inversion.

```
m.inv(array);
```

Input:

- *array* - is a dict whose format follows the rules given in *mln_matrix_mul*.

Return value:

- A dict of the inverse matrix.

Error:

- An error occurred if encounter an invalid argument.

Example:

```
m = Import('matrix');
sys = Import('sys');

array = ['row': 2, 'col': 2, 'data':[1,0,1,1]];
res_array = m.inv(array);
sys.print(res_array['row']);
sys.print(res_array['col']);
sys.print(res_array['data']);
```

The output is:

```
2
2
[1.000000, 0.000000, -1.000000, 1.000000, ]
```

