### Pipe



Pipe provides an approach to send a data set from C layer to script layer.



#### Functions



##### In C

```c
int mln_lang_ctx_pipe_send(mln_lang_ctx_t *ctx, char *fmt, ...);
```

This function will send a data set given by `...` to a specified script task by `ctx`.

`fmt` is used for the interpretation of variable parameters, `fmt` supports three characters:

-  `i` integer, which should be an `mln_s64_t` type integer
- `r` real number, which should be of type `double`
- `s` string, which should correspond to the `mln_string_t` pointer parameter

Return value:

- `0` on success
- `-1` fails



#### In Melang

```
pipe(op);
```

This function is used to subscribe, unsubscribe and receive data from C layer.

`op` has three values:

- `subscribe` starts to accept data set.
- `unsubscribe` stop to accept data set.
- `recv` receive data set.

Return value:

- `nil` - not subscribed if call this function with op `recv`.
- `[]` - no data received.
- An array with each element in it is also an array.



#### Example

In C

```c
mln_lang_ctx_pipe_send(ctx, "ir", 1, 3.14);
```

In Melang

```
sys = import('sys');
pipe('subscribe');
sys.print(pipe('recv'));
pipe('unsubscribe');
```

The output is:

```
[[1, 3.14], ]
```

