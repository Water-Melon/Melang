// tls_busy_test.m
//
// Verifies that when one coroutine has a TLS I/O call in flight on a
// given fd, a second coroutine calling tls.send / tls.recv / tls.handshake /
// tls.set_sni / tls.set_verify_host on the same fd returns false
// immediately (without queuing or blocking).
//
// Topology: server coroutine accepts a single TLS connection, then
// sleeps for a long time before echoing.  Client coroutine connects,
// completes the handshake, sends one payload, and spawns a worker
// coroutine.  The worker calls tls.recv on the same fd that the client
// also holds; while the client's recv is in flight (waiting on the
// slow server), the worker's call MUST return false immediately.
//
// Run:
//   melang tls_busy_test.m

sys = Import('sys');
str = Import('str');
tls = Import('tls');

HOST    = 'localhost';
SERVICE = '18446';
TIMEOUT = 10000;

Eval('tls_busy_server.m', nil, false, 'tls_busy_server');
sys.msleep(300);

conf = tls.conf_new('client', nil, nil, nil, nil, false);
fd   = tls.connect(HOST, SERVICE, conf, TIMEOUT);
if (fd == false || fd == nil) {
    sys.print('tls_busy_test: connect failed');
    tls.conf_free(conf);
    sys.exit(1);
} fi
tls.set_sni(fd, HOST);

if (tls.handshake(fd, TIMEOUT) != true) {
    sys.print('tls_busy_test: handshake failed');
    tls.close(fd);
    tls.conf_free(conf);
    sys.exit(1);
} fi

// Send a probe so the server has something to wait on before echoing.
if (tls.send(fd, 'busy-probe', TIMEOUT) != true) {
    sys.print('tls_busy_test: send failed');
    tls.close(fd);
    tls.conf_free(conf);
    sys.exit(1);
} fi

// Hand the fd to a worker that will block in tls.recv waiting for the
// (slow) server echo.  Once the worker is suspended on recv, this
// coroutine attempts a second concurrent send/recv on the same fd.
Eval('tls_busy_worker.m', fd, false, 'tls_busy_worker');
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

// Let the worker complete normally.  Closing the fd here would race the
// worker; the worker closes after its recv returns.
sys.msleep(2000);

tls.conf_free(conf);

if (fails == 0) {
    sys.print('tls_busy_test: PASS');
} else {
    sys.print('tls_busy_test: FAIL (' + sys.str(fails) + ' assertions)');
}
