# Melang tests

Self-contained Melang scripts used to exercise the language runtime
and the bundled `.so` modules.  Each test pairs a driver (`*_test.m`)
with one or more helper coroutines (`*_server.m`, `*_client.m`,
`*_worker.m`) that the driver spawns via `Eval(...)`.

`Eval` resolves filenames relative to the launcher's working
directory, so the simplest way to run any test is to `cd` into this
directory first:

```
cd tests
../bin/melang tls_test.m
```

Each driver script spawns the helpers it needs via `Eval(...)`, so
**only the driver should appear on the `melang` command line**.
Passing the helper a second time would double-spawn it -- the second
copy would race against the first and (for the server helpers)
typically fail to bind the listen port.

The single exception is the handoff test, where the driver and the
client are designed as two cooperating top-level tasks; that test
takes both names on the command line:

```
../bin/melang tls_handoff_test.m tls_handoff_client.m
```

To exercise the multi-threaded path, add `-t=N`:

```
../bin/melang -t=2 tls_test.m
```

## Inventory

### Smoke / micro-benchmarks
- `melang_smoke_tests.m` — sanity checks for the bytecode VM.
- `fib_bench.m` — naive recursive Fibonacci benchmark.

### TLS
- `tls_test.m` + `tls_server.m` — 100-round non-blocking TLS echo,
  driven by two coroutines sharing one event loop.
- `tls_handoff_test.m` + `tls_handoff_client.m` +
  `tls_handoff_worker.m` — proves that an fd returned by
  `tls.accept` on one coroutine can be passed to (and used from) a
  different coroutine via `Eval(..., EVAL_DATA)`.
- `tls_busy_test.m` + `tls_busy_server.m` + `tls_busy_worker.m` —
  proves that a second coroutine on the same fd is rejected
  immediately (no queuing) by `tls.send` / `tls.recv` /
  `tls.handshake` / `tls.set_sni` / `tls.set_verify_host` while
  another coroutine is mid-I/O.

### HTTPS
- `https_test.m` + `https_server.m` — 50-round
  `http.https_request` exchange over self-signed TLS.
- `https_sni_default_test.m` + `https_sni_default_server.m` —
  proves the `sni` default-to-host fires when the caller omits
  `args['sni']`.

## Prerequisites for the TLS / HTTPS tests

- Melang built with TLS support (Melon's `libmelon_static.a` built
  with `--enable-tls`, and Melang's `./configure` resolved
  `[OpenSSL]`).
- A self-signed cert / key at `/tmp/melang_tls_test.{crt,key}`:

```
openssl req -x509 -newkey rsa:2048 -nodes \
  -keyout /tmp/melang_tls_test.key \
  -out    /tmp/melang_tls_test.crt \
  -subj   "/CN=localhost" \
  -days   1
```

- Ports `18443`, `18444`, `18445`, `18446`, `18447` free on
  localhost.  Each test uses a distinct port so the suite can be run
  back-to-back without waiting for `TIME_WAIT` to drain.
