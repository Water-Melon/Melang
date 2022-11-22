### Message Queue

This document introduces a set of functions about message queue.
Message queue is only working in a coroutine group in the same thread.

There are two kinds of message: queue message and topic message.

Melang supports many coroutines to listen the same queue. If message is a queue message, only one coroutine can get this message. If is topic message, every subscribed coroutine can receive this message.



### Import

```
mq = Import('mq');
```



##### subscribe

Subscribe a topic message.

```
mq.subscribe(qname);
```

Input:

- qname - a string queue name.

Return value:

- Always return *nil*.

Error:

- Invalid argument will throw an error.



##### unsubscribe

Unsubscribe a topic.

```
mq.unsubscribe(qname);
```

Input:

- qname - a string queue name.

Return value:

- Always *nil*.

Error:

- Invalid argument will throw an error.



##### send

Send message to a specified queue.

```
mq.send(qname, msg, asTopic);
```

Input:

- qname - a string queue name.
- msg - the message that we want to send. The type of this argument must be *integer, boolean, real or string*.
- asTopic - if want to send queue message, this argument must be *false*, otherwise *true*. This is an optional argument. Omitted means *false*.

Return value:

- Always *nil*.

Error:

- Invalid argument will throw an error.



##### recv

```
mq.recv(qname, timeout);
```

Input:

- qname - a string queue name.
- timeout - a positive integer or *nil*. If is positive integer, program will wait for *timeout* microseconds. If omitted or *nil*, prigram will wait until got a message.

Return value:

- A message.

Error:

- Invalid argument will throw an error.



##### Example

```
//file a.mln
mq = Import('mq');

mq.send('test', 'hello');
```

```
//b.mln
mq = Import('mq');
sys = Import('sys');

msg = mq.recv('test');
sys.print(msg);
```

```
$ melang a.mln b.mln
```

The output is:

```
hello
```

