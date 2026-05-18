### TLS

This document introduces a set of functions for non-blocking TLS in Melang.

The TLS module wraps the TLS support that the underlying Melon C
library exposes through `mln_tcp_conn`, so Melang scripts can do
TLS exactly the way they already do plain TCP through the
[`net`](https://water-melon.github.io/Melang/lib/network.html)
module: every call **looks** synchronous, but I/O that would block
yields the current coroutine to the scheduler so other coroutines
keep running.

Requirements:

- The Melon C library must be built with `--enable-tls` so
  `libmelon_static.a` exports the `mln_tcp_conn_tls_*` symbols.
- Melang must be configured with `--enable-tls` (see the launcher
  page) so `tls.so` (and `http.so`'s `https_request` branch) is
  built and links against the host OpenSSL.



### Import

```
tls = Import('tls');
```



### Concurrency model

A TLS fd returned by `tls.accept` or `tls.connect` lives in the
per-`mln_lang_t` connection table.  Once accept / connect has
returned, the fd is **not** owned by the originating coroutine;
any coroutine that holds the integer can call `tls.send / recv /
handshake / close` on it (a common pattern is to spawn a worker
via `Eval('handler.m', cfd, false, ...)` and hand the fd off).

While one coroutine has a `send`, `recv`, `handshake`, or
`shutdown` in flight on a given fd, a call from a second
coroutine on the same fd **returns immediately**:

- `tls.send`, `tls.recv`, `tls.handshake` -> return `false` and
  set the error message *"Socket busy in another script task."*.
- `tls.close` always succeeds (it simply drops the resource).

The second coroutine is **not** queued or blocked.  If you want
the second coroutine to wait, gate the call with `mq` or another
synchronisation primitive at the script level.  Most code will
not need this, because the natural pattern is "one coroutine
owns one connection".

Under `melang -t=N` the connection table is shared across all
worker threads.  All accesses to it happen under the lang
mutex, and individual `SSL` objects are serialised by the same
busy-flag check above, so OpenSSL's "one thread per SSL at a
time" requirement is upheld.  Configuration objects
(`SSL_CTX` wrappers built by `tls.conf_new`) are internally
thread-safe in OpenSSL >= 1.1.0 and may be shared across
coroutines and across threads.



##### conf_new

Build a reusable TLS configuration object.  An `mln_tcp_tls_conf_t`
in C terms — wraps an `SSL_CTX` plus the certificate / key / CA file
paths.  Cheap to share across many connections; expensive to build,
so callers are expected to allocate once and reuse.

```
tls.conf_new(role, cert, key, ca, ciphers, verify);
```

Input:

- `role` - a string, must be either `'server'` or `'client'`.
- `cert` - a string path to the PEM certificate file, or `nil`.
  Required for `role='server'`; optional for `role='client'`.
- `key` - a string path to the PEM private key file, or `nil`.
  Required for `role='server'`; optional for `role='client'`.
- `ca` - a string path to a PEM CA bundle used to verify the peer,
  or `nil`.
- `ciphers` - a string passed to `SSL_CTX_set_cipher_list` (TLSv1.2
  cipher list), or `nil` for the OpenSSL default.
- `verify` - a boolean.  If `true`, the peer certificate is verified
  against the CA bundle; on the server side the peer is also
  required to present a certificate.

Return value:

- A positive integer (a conf handle) on success.  Pass this handle
  to `tls.accept` / `tls.connect`.
- `false` on failure (bad file, bad cipher list, OpenSSL error).



##### conf_free

Release a configuration object built by `tls.conf_new`.

```
tls.conf_free(conf);
```

Input:

- `conf` - the integer handle returned by `tls.conf_new`.

Return value:

- Always `nil`.

After this call, any connection still using the conf will keep
working (the underlying `SSL_CTX` is reference-counted by OpenSSL),
but the handle itself becomes invalid.  Do not free a conf while
new accept / connect calls might still race to use the handle.



##### listen

Set up a plain TCP listen socket.  The same call as
`net.tcp_listen` — TLS is layered on later, in `tls.accept`.

```
tls.listen(host, service);
```

Input:

- `host` - a string IP or domain name.
- `service` - a string port number or protocol name, e.g. `'443'`,
  `'https'`.

Return value:

- A listen file descriptor on success.
- `false` on failure.



##### accept

Accept a single incoming TCP connection on a listen fd and wrap it
in a TLS server-side connection using the given conf.  The handshake
is **not** driven by this call — call `tls.handshake` next, or just
issue `tls.recv` / `tls.send` and the handshake will run on demand.

```
tls.accept(fd, conf, timeout);
```

Input:

- `fd` - a listen fd returned by `tls.listen`.
- `conf` - a server-role conf handle from `tls.conf_new`.
- `timeout` - milliseconds; `nil` means wait forever.

Return value:

- A new TLS file descriptor on success.  Use this fd as the
  argument to subsequent `tls.send` / `tls.recv` / `tls.close` /
  `tls.handshake` calls.
- `nil` on timeout.
- `false` on any other failure.



##### connect

Open a TCP connection to a remote peer and wrap it in a TLS client
connection using the given conf.  Like `accept`, the TLS handshake
is **not** driven here; use `tls.handshake` (or just `tls.send` /
`tls.recv`) afterwards.

```
tls.connect(host, service, conf, timeout);
```

Input:

- `host` - a string IP or domain name.
- `service` - a string port number or protocol name.
- `conf` - a client-role conf handle from `tls.conf_new`.
- `timeout` - milliseconds; `nil` means wait forever.

Return value:

- A new TLS file descriptor on success.
- `nil` on timeout.
- `false` on any other failure.



##### set_sni

Set the SNI (Server Name Indication) hostname on a client-side TLS
fd before the handshake starts.  Required by many virtual-hosted
servers.

```
tls.set_sni(fd, hostname);
```

Input:

- `fd` - a TLS fd returned by `tls.connect`.
- `hostname` - a string (no embedded NUL bytes; ≤ 253 bytes per
  RFC 1035).

Return value:

- `true` on success.
- `false` on failure (bad fd, oversize / NUL-bearing hostname).



##### set_verify_host

Enable RFC 6125 hostname verification against the peer certificate.
Call before the handshake.

```
tls.set_verify_host(fd, hostname);
```

Input:

- `fd` - a TLS fd returned by `tls.connect`.
- `hostname` - the hostname that the peer certificate's CN / SAN
  must match.

Return value:

- `true` on success.
- `false` on failure.

Verification only kicks in when the conf was built with `verify=true`
and a CA bundle was provided.



##### handshake

Drive the TLS handshake explicitly.  Optional: the first `send` or
`recv` call will trigger it automatically.  Calling it explicitly
lets the script surface a handshake-failure before the first I/O,
which is often more readable in test code.

```
tls.handshake(fd, timeout);
```

Input:

- `fd` - a TLS fd from `tls.accept` or `tls.connect`.
- `timeout` - milliseconds; `nil` for no timeout.

Return value:

- `true` on completion.
- `nil` on timeout.
- `false` on failure.



##### send

Encrypt the buffer and push it to the peer.  Yields the coroutine
while waiting on the socket; other coroutines keep running.

```
tls.send(fd, data, timeout);
```

Input:

- `fd` - a TLS fd from `tls.accept` or `tls.connect`.
- `data` - a string.  An empty string is a no-op and returns `true`
  immediately.
- `timeout` - milliseconds; `nil` for no timeout.

Return value:

- `true` on success (all bytes encrypted and on the wire).
- `nil` on timeout.
- `false` on error / connection closed.

Small payloads typically complete inline without an event-loop
trip; only when the socket buffer is full does the script suspend.



##### recv

Receive and decrypt a chunk of plaintext from the peer.  Coalesces
whatever plaintext is currently pending (one or more TLS records)
into a single Melang string.

```
tls.recv(fd, timeout);
```

Input:

- `fd` - a TLS fd from `tls.accept` or `tls.connect`.
- `timeout` - milliseconds; `nil` for no timeout.

Return value:

- A non-empty string on success.
- `true` if the peer cleanly closed the connection.
- `nil` on timeout.
- `false` on error.

For framed protocols on top of TLS the caller must loop until they
have accumulated enough bytes; `recv` is purely a chunk delivery
primitive.



##### close

Send a TLS close_notify alert (best-effort, non-blocking) and close
the underlying socket.

```
tls.close(fd);
```

Input:

- `fd` - a fd returned by `tls.listen`, `tls.accept` or `tls.connect`.

Return value:

- Always `nil`.

Scripts that need a strict bidirectional close_notify should call
`tls.recv` until it returns `true` before calling `tls.close`.



##### Example

A minimal echo over TLS using two coroutines on one event loop.

Generate a self-signed cert/key once:

```
openssl req -x509 -newkey rsa:2048 -nodes \
  -keyout /tmp/melang_tls_test.key \
  -out    /tmp/melang_tls_test.crt \
  -subj   "/CN=localhost" \
  -days   1
```

```
//tls_server.m
sys = Import('sys');
tls = Import('tls');

conf = tls.conf_new('server',
                    '/tmp/melang_tls_test.crt',
                    '/tmp/melang_tls_test.key',
                    nil, nil, false);
lfd  = tls.listen('localhost', '18443');
cfd  = tls.accept(lfd, conf, 10000);
tls.handshake(cfd, 10000);

while (1) {
    msg = tls.recv(cfd, 10000);
    if (msg == false || msg == nil || msg == true) { break; } fi
    tls.send(cfd, msg, 10000);
}

tls.close(cfd);
tls.close(lfd);
tls.conf_free(conf);
```

```
//tls_client.m
sys = Import('sys');
tls = Import('tls');

sys.msleep(300);

conf = tls.conf_new('client', nil, nil, nil, nil, false);
fd   = tls.connect('localhost', '18443', conf, 10000);
tls.set_sni(fd, 'localhost');
tls.handshake(fd, 10000);

tls.send(fd, 'hello-from-melang', 10000);
sys.print(tls.recv(fd, 10000));

tls.close(fd);
tls.conf_free(conf);
```

Run both as siblings on a single Melang scheduler:

```
$ melang tls_server.m tls_client.m
```

The output is:

```
hello-from-melang
```
