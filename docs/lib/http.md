### HTTP

This HTTP library does **not** support chunk mode.



### Import

```
http = Import('http');
```



##### parse

```
http.parse(data);
```

Input:

- `data` - a raw HTTP string.

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

- `data` - an array (dict) which is used to be constructed an HTTP string.

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



##### https_request

End-to-end HTTPS client.  Available only when Melon was built with
`--enable-tls` and Melang's `./configure` resolved `TLS  [OpenSSL]`
(auto-detected from `libmelon_static.a`; see the
[TLS module](https://water-melon.github.io/Melang/lib/tls.html)
for the full configure surface).
The call drives the whole exchange — TCP connect, TLS handshake,
request build (via `http.create`), encrypted send, encrypted recv,
response parse (via `http.parse`) — in a single C state machine,
suspending the script across each I/O wait so other coroutines on
the same scheduler keep running.

```
http.https_request(args);
```

Input:

- `args` - an array (dict) describing the request.  Recognised keys:
  - `host` (required string) - server hostname or IP.
  - `service` (required string) - port or service name, e.g.
    `'443'`, `'https'`.
  - `request` (required array) - request dict in the same shape as
    `http.create` expects (`type`, `method`, `version`, `uri`,
    `args`, `headers`, `body`).
  - `timeout` (optional int / nil) - per-step timeout in
    milliseconds.  `nil` (or omitted) means wait forever.
  - `ca` (optional string) - path to a PEM CA bundle for peer
    verification.  Ignored if `conf` is supplied.
  - `verify` (optional bool / int / nil) - require peer certificate
    verification.  A non-zero integer is treated as `true`; `nil` or
    a missing key means "don't enable verification" (the same as
    `false`), so the default when the caller doesn't care is "off".
    Ignored if `conf` is supplied.  Any other type (e.g. a string)
    is rejected with `args.verify must be a bool, int, or nil.`
  - `sni` (optional string) - SNI hostname.  Defaults to `host`
    when `host` is a DNS name; the default is suppressed when
    `host` is an IP literal because RFC 6066 forbids IP SNI.
  - `verify_host` (optional string) - hostname to match the peer
    certificate against (RFC 6125).  Defaults to `host` when
    `verify=true` and `host` is a DNS name; the default is
    suppressed for IP literals.
  - `conf` (optional int) - a pre-built TLS conf handle from
    `tls.conf_new`.  When provided, the call reuses the existing
    `SSL_CTX` instead of allocating one per request, which is
    significantly faster for repeated calls.

Return value:

- A response dict in the same shape as `http.parse` returns
  (`type='response'`, `version`, `code`, `headers`, `body`) on
  success.
- `nil` on timeout.
- `false` on any other failure (DNS, connect, TLS, parse).

Errors:

- Missing or wrong-typed required arguments throw an error and
  terminate the current coroutine; other coroutines continue.



##### Example: https_request

```
sys  = Import('sys');
http = Import('http');

resp = http.https_request([
    'host':    'example.com',
    'service': '443',
    'sni':     'example.com',
    'verify':  false,
    'timeout': 15000,
    'request': [
        'type':    'request',
        'method':  'GET',
        'version': '1.1',
        'uri':     '/',
        'args':    '',
        'headers': [
            'Host':           'example.com',
            'User-Agent':     'melang-https',
            'Content-Length': '0',
        ],
        'body':    '',
    ],
]);

if (resp == false || resp == nil) {
    sys.print('HTTPS request failed');
} else {
    sys.print(resp['code']);
    sys.print(resp['body']);
} fi
```



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

