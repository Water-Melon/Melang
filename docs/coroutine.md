### Coroutine

Coroutine in Melang is time-sharing scheduled in a single thread.

If you use the execution file which is generated by the repository on Github, script files in a same melang command will be executed in one process or even one thread.

e.g.

```
/*the tasks those behind command name are running in a same thread.*/
$ ./melang a.mln b.mln ...
```



Besides this way, there is a function named `Eval` to start a new script task in the current thread to execute a piece of code.

```
Eval(val, data, in_string, alias);
```

If `in_string` is true, `val` will be the script code, otherwise `val` is the script file path.

`data` will be passed to the new script task. If we want to use `data` in new task, we can use the variable named `EVAL_DATA` which is a built-in variable added by `Eval`.

`data` not support all data types, it just supports:

- integer
- real
- boolean
- nil
- string

`alias` is the name to be indicated to the new coroutine task, and it can be omitted if it doesn't need a name.

If `val` is `nil`, function will return an array of names of all named coroutines.

e.g.

```
sys = Import('sys');

Eval('sys = Import('sys'); while (1) {sys.print(EVAL_DATA);}', 'bbb', true);
while (1) {
  sys.print('aaa');
}
```

The output would be like:

```
...
aaa
aaa
bbb
bbb
bbb
bbb
bbb
bbb
aaa
...
```

There is the same problem as shown in [preprocess](https://water-melon.github.io/Melang/preprocess.html). If `val` is a file path, it will follow the same rules as `include`'s.



Melang also provide a function to kill running coroutine task.

```
Kill(alias);
```

`alias` is the name that given by function `Eval`.

This function always return `nil`.




#### Examples

Here are two comprehensive examples of HTTP server. There are some functions which will be introduced in [library](	https://water-melon.github.io/Melang/library.html) used in these two examples.



There are two files: `server.mln` and `worker.mln`.

Example 1.

Each TCP connection will be handled in an individual coroutine.

```
/* filename: server.mln */
net = Import('net');
sys = Import('sys');

listenfd = net.tcp_listen('127.0.0.1', '80');
while (1) {
    fd = net.tcp_accept(listenfd);
    sys.print(fd);
    Eval('worker.mln', fd);
}
```

```
/* filename: worker.mln */
net = Import('net');
sys = Import('sys');

fd = EVAL_DATA;
ret = net.tcp_recv(fd);
if (ret) {
    net.tcp_send(fd, "HTTP/1.1 200 OK\r\nContent-Length: 1\r\n\r\na\r\n\r\n");
}fi
net.tcp_close(fd);
sys.print('quit');
```

Now, you can use `ab (apache bench)` to test.



Example 2.

Create a coroutine pool which has 100 coroutines.

Each TCP will be delivered to a coroutine which is in coroutine pool to be handled.

```
/* filename: server.mln */
net = Import('net');
mq = Import('mq');

listenfd = net.tcp_listen('127.0.0.1', '80');
for (i = 0; i < 4; ++i) {
    Eval('worker.mln', i);
}
while (1) {
    fd = net.tcp_accept(listenfd);
    mq.send('test', fd);
}
```

```
/* filename: worker.mln */
sys = Import('sys');
net = Import('net');
mq = Import('mq');

sys.print(EVAL_DATA);
while (1) {
    fd = mq.recv('test');
    ret = net.tcp_recv(fd);
    if (ret) {
        net.tcp_send(fd, "HTTP/1.1 200 OK\r\nContent-Length: 1\r\n\r\na\r\n\r\n");
    }fi
    net.tcp_close(fd);
}
```
