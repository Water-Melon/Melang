// tls_busy_test.m
//
// Self-contained.  Verifies that when one coroutine has a TLS I/O
// call in flight on a given fd, a second coroutine calling
// tls.send / tls.recv / tls.handshake / tls.set_sni /
// tls.set_verify_host on the same fd returns false immediately,
// without queuing or blocking.
//
// Topology:
//   - server coroutine: accepts a single TLS connection, recv()s
//     one payload, sleeps ~1.5s, then echoes -- giving the client
//     a stable window in which to attempt the second concurrent op.
//   - main coroutine = client: connects, completes handshake,
//     sends one probe, spawns a worker that holds a recv in flight,
//     then attempts a second concurrent send/recv/handshake/
//     set_sni/set_verify_host on the same fd.  Each MUST return
//     false immediately.
//   - worker coroutine: holds recv on the shared fd until the
//     server's slow echo arrives, then closes.
//
// Run (from the tests/ directory):
//   ../bin/melang tls_busy_test.m
//   ../bin/melang -t=2 tls_busy_test.m

sys = Import('sys');
str = Import('str');
tls = Import('tls');

HOST    = 'localhost';
SERVICE = '18446';
TIMEOUT = 10000;

// Inline server: slow-echo so the client has a stable window.
server_src = "sys = Import('sys');
tls = Import('tls');
conf = tls.conf_new('server',
                    '/tmp/melang_tls_test.crt',
                    '/tmp/melang_tls_test.key',
                    nil, nil, false);
if (!conf) {
    sys.print('tls_busy_server: conf_new failed');
    return;
} fi
lfd = tls.listen('localhost', '18446');
if (lfd == false) {
    sys.print('tls_busy_server: listen failed');
    tls.conf_free(conf);
    return;
} fi
cfd = tls.accept(lfd, conf, 10000);
if (cfd == false || cfd == nil) {
    sys.print('tls_busy_server: accept failed');
    tls.close(lfd);
    tls.conf_free(conf);
    return;
} fi
if (tls.handshake(cfd, 10000) != true) {
    sys.print('tls_busy_server: handshake failed');
    tls.close(cfd);
    tls.close(lfd);
    tls.conf_free(conf);
    return;
} fi
msg = tls.recv(cfd, 10000);
sys.msleep(1500);
tls.send(cfd, msg, 10000);
tls.close(cfd);
tls.close(lfd);
tls.conf_free(conf);
";

// Inline worker: blocks in tls.recv on the shared fd, so the
// parent coroutine can confirm that the busy-flag rejects a
// second concurrent op while this recv is in flight.
worker_src = "sys = Import('sys');
tls = Import('tls');
fd = EVAL_DATA;
tls.recv(fd, 10000);
tls.close(fd);
";

Eval(server_src, nil, true, 'tls_busy_server');
sys.msleep(300);

conf = tls.conf_new('client', nil, nil, nil, nil, false);
fd   = tls.connect(HOST, SERVICE, conf, TIMEOUT);
if (fd == false || fd == nil) {
    sys.print('tls_busy_test: connect failed');
    tls.conf_free(conf);
    return;
} fi
tls.set_sni(fd, HOST);

if (tls.handshake(fd, TIMEOUT) != true) {
    sys.print('tls_busy_test: handshake failed');
    tls.close(fd);
    tls.conf_free(conf);
    return;
} fi

// Send a probe so the server has something to wait on before echoing.
if (tls.send(fd, 'busy-probe', TIMEOUT) != true) {
    sys.print('tls_busy_test: send failed');
    tls.close(fd);
    tls.conf_free(conf);
    return;
} fi

// Hand the fd to a worker that will block in tls.recv waiting for
// the (slow) server echo.  Once the worker is suspended on recv,
// this coroutine attempts a second concurrent send / recv / etc.
Eval(worker_src, fd, true, 'tls_busy_worker');
sys.msleep(200);

// Second send while worker.recv is in flight -- must fail immediately.
busy_send = tls.send(fd, 'should-be-rejected', TIMEOUT);
busy_recv = tls.recv(fd, TIMEOUT);
busy_hs   = tls.handshake(fd, TIMEOUT);
busy_sni  = tls.set_sni(fd, HOST);
busy_vh   = tls.set_verify_host(fd, HOST);

fails = 0;
if (busy_send != false) {
    sys.print('tls_busy_test: send was not rejected (got: ' + sys.str(busy_send) + ')');
    fails = fails + 1;
} fi
if (busy_recv != false) {
    sys.print('tls_busy_test: recv was not rejected (got: ' + sys.str(busy_recv) + ')');
    fails = fails + 1;
} fi
if (busy_hs != false) {
    sys.print('tls_busy_test: handshake was not rejected (got: ' + sys.str(busy_hs) + ')');
    fails = fails + 1;
} fi
if (busy_sni != false) {
    sys.print('tls_busy_test: set_sni was not rejected (got: ' + sys.str(busy_sni) + ')');
    fails = fails + 1;
} fi
if (busy_vh != false) {
    sys.print('tls_busy_test: set_verify_host was not rejected (got: ' + sys.str(busy_vh) + ')');
    fails = fails + 1;
} fi

// Let the worker complete normally.  Closing the fd here would race
// the worker; the worker closes after its recv returns.
sys.msleep(2000);

tls.conf_free(conf);

if (fails == 0) {
    sys.print('tls_busy_test: PASS');
} else {
    sys.print('tls_busy_test: FAIL (' + sys.str(fails) + ' assertions)');
}
