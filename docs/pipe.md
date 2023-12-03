### Pipe



The pipe provides a way for the C level to communicate with the script level.



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
- `-1` on failure



```c
int mln_trace_recv_handler_set(mln_lang_ctx_pipe_recv_cb_t recv_handler);

typedef int (*mln_lang_ctx_pipe_recv_cb_t)(mln_lang_ctx_t *, mln_lang_val_t *);
```

This function is used to receive data from the script level in C code.

The parameters of `recv_handler`:

- the first one is the script task context object.
- the second one is the data sent from script level.

Return value:

- `0` on succes
- `-1` on failure



#### In Melang

```
Pipe(op);
```

This function is used to subscribe, unsubscribe and receive data from C layer.

`op` has three values:

- `subscribe` starts to accept data set.
- `unsubscribe` stop to accept data set.
- `recv` receive data set.
- `send` send data to C level.

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
sys = Import('sys');
Pipe('subscribe');
sys.print(Pipe('recv'));
Pipe('unsubscribe');
```

The output is:

```
[[1, 3.14], ]
```

