### Prime Number



### Import

```
p = import('prime');
```



##### prime

Generate a prime number greater than or equal to a given number.

```
p.prime(base);
```

Input:

- base - an integer number.

Return value:

- a prime number.

Error:

- Invalid argument will throw an error.

Example:

```
p = import('prime');
sys = import('sys');

sys.print(p.prime(22));
```

The output is:

```
23
```

