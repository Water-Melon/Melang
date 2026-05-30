# Melang tests

Self-contained Melang scripts used to exercise the language runtime
and the bundled `.so` modules.  Every test (except `https_server.m`,
which is a long-running example server) is fully self-contained:
the driver script spawns its own server / worker / client coroutines
on the same scheduler via `Eval(src, data, true, alias)` (inline
source mode), so a single `melang foo_test.m` invocation runs the
whole test end to end.

`Eval` resolves relative filenames against the launcher's working
directory, but the tests don't depend on that any more — each test
inlines the source of its helpers.  Run from the `tests/` directory
just for convenience:

```
cd tests
../bin/melang tls_test.m
```

To exercise the multi-threaded path, add `-t=N`:

```
../bin/melang -t=2 tls_test.m
```

## Inventory

### Smoke / micro-benchmarks
- `melang_smoke_tests.m` — sanity checks for the bytecode VM
  (arithmetic, control flow, closures, `Set`/objects, `Watch`,
  `Eval`, and a handful of tagged-value-stack regressions).
- `fib_bench.m` — naive recursive Fibonacci benchmark.

### Security regressions
- `network_overflow_test.m` — regression for the `net.udp_send`
  `service` stack buffer overflow (the length guard compared against
  the host buffer size instead of the service buffer size).  A fixed
  build rejects the oversized argument and exits 0; a vulnerable build
  aborts with a buffer-overflow / stack-smashing diagnostic.

### TLS
- `tls_test.m` — 100-round non-blocking TLS echo.  Inline server
  + main-coroutine client on one scheduler.
- `tls_handoff_test.m` — proves that an fd returned by
  `tls.accept` on one coroutine can be passed to (and used from)
  a different coroutine via `Eval(..., EVAL_DATA, true)`.  Inline
  worker (accepts the fd) + inline client + main-coroutine
  server, all in one file.
- `tls_busy_test.m` — proves that a second coroutine on the same
  fd is rejected immediately (no queuing) by `tls.send` /
  `tls.recv` / `tls.handshake` / `tls.set_sni` /
  `tls.set_verify_host` while another coroutine is mid-I/O.
  Inline slow-echo server + inline blocking worker + main-coroutine
  client.

### HTTPS
- `https_test.m` — 50-round `http.https_request` exchange over
  self-signed TLS.  Inline server + main-coroutine client.
- `https_sni_default_test.m` — proves the `sni` default-to-host
  fires when the caller omits `args['sni']`.  Inline single-shot
  server + main-coroutine `http.https_request` call.
- `https_server.m` — **long-running standalone example server**
  (the only non-self-contained `.m` here).  Listens on port
  18444, hands every accepted connection off to a per-connection
  inline worker, and answers a 200 OK that echoes the request.
  Designed to be driven by `curl` / `ab` / `wrk`, not by another
  Melang script.  See the file header for `curl` and `ab`
  invocations.

## Prerequisites for the TLS / HTTPS tests

- Melon built with `--enable-tls`; Melang's `./configure` then
  auto-detects it (the summary should read `TLS  [OpenSSL]`).
- A self-signed cert / key at `/tmp/melang_tls_test.{crt,key}`.
  The tests run as `localhost`, but the standalone `https_server.m`
  is also useful from `https://127.0.0.1:...`, so generate the
  cert with both DNS and IP SANs so `curl` can verify either:

```
openssl req -x509 -newkey rsa:2048 -nodes \
  -keyout /tmp/melang_tls_test.key \
  -out    /tmp/melang_tls_test.crt \
  -subj   "/CN=localhost" \
  -addext "subjectAltName=DNS:localhost,IP:127.0.0.1,IP:::1" \
  -days   1
```

- Ports `18443`, `18444`, `18445`, `18446`, `18447` free on
  localhost.  Each test uses a distinct port so the suite can be
  run back-to-back without waiting for `TIME_WAIT` to drain.

## Driving `https_server.m` with curl

The example server uses a self-signed certificate, so curl needs
either `--cacert` (recommended) or `-k` (skip verification):

```
../bin/melang https_server.m &
curl --cacert /tmp/melang_tls_test.crt https://localhost:18444/
curl --cacert /tmp/melang_tls_test.crt https://127.0.0.1:18444/
curl -k https://127.0.0.1:18444/        # any-hostname, skip verify
ab  -n 1000 -c 16 https://localhost:18444/
```

A "handshake failed" with a plain `curl https://...` (no `-k` or
`--cacert`) is curl rejecting the self-signed cert during
verification — not a server bug.  Either trust the cert via
`--cacert` or skip verification via `-k`.
