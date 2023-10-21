### HTTP

This HTTP library does not support chunk mode.



### Import

```
http = Import('http');
```



##### parse

```
http.parse(data);
```

Input:

- data - a raw HTTP string.

Return value:

- a parsed HTTP array.

  Format (example):

  - request

    ```
    [
      "type": "request",
      "method": "GET",
      'version': '1.1',
        'uri': '/',
        'args': 'a=1&b=2',
        'headers': [
          "Referer": "melang.org",
        ],
        'body': 'abc',
    ]
    ```

  - response

    ```
    [
      'type': 'response',
      'version': '1.1',
      'code': 200,
      'headers': [
        "Server": "Melang",
      ],
      'body': 'cba',
    ]
    ```

Error:

- Invalid argument will throw an error.



##### create

```
http.create(data);
```

Input:

- data - an array (dict) which is used to be constructed an HTTP string.

  Format (example):

  - request

    ```
    [
      "type": "request",
      "method": "GET",
      'version': '1.1',
        'uri': '/',
        'args': 'a=1&b=2',
        'headers': [
          "Referer": "melang.org",
        ],
        'body': 'abc',
    ]
    ```

  - response

    ```
    [
      'type': 'response',
      'version': '1.1',
      'code': 200,
      'headers': [
        "Server": "Melang",
      ],
      'body': 'cba',
    ]
    ```

Return value:

- a constructed HTTP string.

Error:

- Invalid argument will throw an error.



##### Example 1

```
h = Import('http');
sys = Import('sys');

sys.print(h.parse("GET /?a=1&b=2 HTTP/1.1\r\nContent-Length:3\r\nReferer: melonc.io\r\n\r\nabc\r\n"));
sys.print(h.parse("HTTP/1.1 200 OK\r\nServer: Apache\r\nContent-Length:3\r\n\r\ncba\r\n"));

sys.print(h.create([
    'type': 'request',
    'method': 'GET',
    'version': '1.1',
    'uri': '/',
    'args': 'a=1',
    'headers': [
      "Referer": "melonc.io",
    ],
    'body': 'abc',
]));
sys.print(h.create([
    'type': 'response',
    'version': '1.1',
    'code': 200,
    'headers': [
      "Server": "Melang",
    ],
    'body': 'cba',
]));
```

The output is:

```
[request, GET, 1.1, /, a=1&b=2, [3, melonc.io, ], abc, ]
[response, 1.1, 200, [Apache, 3, ], cba, ]
GET /?a=1 HTTP/1.1
Content-Length: 3
Referer: melonc.io

abc
HTTP/1.1 200 OK
Server: Melang
Content-Length: 3

cba
```



##### Example 2

```
//server.m

net = Import('net');
mq = Import('mq');
sys = Import('sys');

listenfd = net.tcp_listen('127.0.0.1', '1234');
for (i = 0; i < 2; ++i) {
    Eval('worker.m', i);
}
while (1) {
    fd = net.tcp_accept(listenfd);
    if (!(sys.is_int(fd)))
        continue;
    fi
    mq.send('test', fd);
}
```

```
//worker.m

#include "@/index.m"

sys = Import('sys');
net = Import('net');
mq = Import('mq');
http = Import('http');
str = Import('str');

sys.print(EVAL_DATA);
while (1) {
    fd = mq.recv('test');
    ret = net.tcp_recv(fd);
    if (ret && sys.is_str(ret)) {
        R = http.parse(ret);
        R['type'] = 'response';
        R['headers'] = [];

        uri = str.slice(R['uri'], '/');
        uri && (ctlr = str.capitalize(uri[0]), o = $ctlr);
        if (!o || sys.has(o, uri[1]) != 'method') {
            R['code'] = 404;
        } else {
            o.__action__ = uri[1];
            R['body'] = o.__action__();
        }

        net.tcp_send(fd, http.create(R));
    }fi
    net.tcp_close(fd);
}
```

```
//index.m

Json = Import('json');

Index {
    @index() {
        R['headers']['Content-Type'] = 'application/json';
        return Json.encode(['code': 200, 'msg': 'OK']);
    }
}
```

Then start up HTTP server:

```
$ melang server.m
```

Send an HTTP request by `curl`

```
$ curl -v http://127.0.0.1:1234/index/index
```

The output is:

```
*   Trying 127.0.0.1:1234...
* Connected to 127.0.0.1 (127.0.0.1) port 1234 (#0)
> GET /index/index HTTP/1.1
> Host: 127.0.0.1:1234
> User-Agent: curl/7.81.0
> Accept: */*
> 
* Mark bundle as not supporting multiuse
< HTTP/1.1 200 OK
< Content-Length: 23
< Content-Type: application/json
< 
* Connection #0 to host 127.0.0.1 left intact
{"code":200,"msg":"OK"}
```

