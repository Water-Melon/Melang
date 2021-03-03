### Message Queue

This document introduces a set of functions about message queue.
Message queue is only working in a coroutine group in the same thread.

There are two kinds of message: queue message and topic message.

Melang supports many coroutines to listen the same queue. If message is a queue message, only one coroutine can get this message. If is topic message, every subscribed coroutine can receive this message.



##### mln_msg_topic_subscribe

Subscribe a topic message.

```
@mln_msg_topic_subscribe(qname);
```

Input:

- qname - a string queue name.

Return value:

- Always return *nil*.

Error:

- Invalid argument will throw an error.



##### mln_msg_topic_unsubscribe

Unsubscribe a topic.

```
@mln_msg_topic_unsubscribe(qname);
```

Input:

- qname - a string queue name.

Return value:

- Always *nil*.

Error:

- Invalid argument will throw an error.



##### mln_msg_queue_send

Send message to a specified queue.

```
@mln_msg_queue_send(qname, msg, asTopic);
```

Input:

- qname - a string queue name.
- msg - the message that we want to send. The type of this argument must be *integer, boolean, real or string*.
- asTopic - if want to send queue message, this argument must be *false*, otherwise *true*. This is an optional argument. Omitted means *false*.

Return value:

- Always *nil*.

Error:

- Invalid argument will throw an error.



##### mln_msg_queue_recv

```
@smln_msg_queue_recv(qname, timeout);
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
@mln_msg_queue_send('test', 'hello');
```

```
//b.mln
msg = @mln_msg_queue_recv('test');
@mln_print(msg);
```

```
$ melang a.mln b.mln
```

The output is:

```
hello
```

