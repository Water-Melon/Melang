// tls_handoff_test.m
//
// Self-contained.  Proves that the fd returned by tls.accept is fully
// portable across coroutines: the accepting coroutine hands the fd
// off to a worker via Eval(..., EVAL_DATA, true) and exits without
// touching the fd again.  The worker drives handshake + N-round
// echo on the inherited fd, the client coroutine drives the other
// side of the echo from a fresh tls.connect.
//
// Topology:
//   - main coroutine = server: listens, accepts ONE TLS connection,
//     spawns "worker" inline with the accepted fd as EVAL_DATA, then
//     exits.  This exercises the cross-coroutine handoff and the
//     cleanup path when the accepting coroutine terminates while
//     the fd is still in use.
//   - worker coroutine: handshakes + echoes on the inherited fd.
//   - client coroutine: connects on the other side and drives the
//     same N-round echo loop.
//
// Requirements: same as tls_test.m, plus port 18445 free.
//
// Run (from the tests/ directory):
//   ../bin/melang tls_handoff_test.m
//   ../bin/melang -t=2 tls_handoff_test.m

sys = Import('sys');
tls = Import('tls');

HOST    = 'localhost';
SERVICE = '18445';
TIMEOUT = 10000;
CERT    = '/tmp/melang_tls_test.crt';
KEY     = '/tmp/melang_tls_test.key';

// Worker coroutine -- receives accepted fd via EVAL_DATA, drives
// the handshake + 30-round echo, and reports the verdict.
worker_src = "sys = Import('sys');
str = Import('str');
tls = Import('tls');
fd = EVAL_DATA;
if (tls.handshake(fd, 10000) != true) {
    sys.print('worker: handshake failed');
    tls.close(fd);
    return;
} fi
i = 0;
fails = 0;
while (i < 30) {
    out = 'handoff=' + sys.str(i);
    if (tls.send(fd, out, 10000) != true) {
        sys.print('worker: send failed at ' + sys.str(i));
        fails = fails + 1;
        break;
    } fi
    expected = str.strlen(out);
    got = '';
    recv_failed = false;
    while (str.strlen(got) < expected) {
        chunk = tls.recv(fd, 10000);
        if (chunk == false || chunk == nil || chunk == true) {
            sys.print('worker: recv failed at ' + sys.str(i));
            recv_failed = true;
            break;
        } fi
        got = got + chunk;
    }
    if (recv_failed) { fails = fails + 1; break; } fi
    if (str.strlen(got) != expected || !str.strcmp(got, out)) {
        sys.print('worker: mismatch at ' + sys.str(i));
        fails = fails + 1;
        break;
    } fi
    sys.print('handoff_test: round ' + sys.str(i) + ' ok');
    i = i + 1;
}
tls.close(fd);
if (fails == 0 && i == 30) {
    sys.print('handoff_test: PASS (30 rounds)');
} else {
    sys.print('handoff_test: FAIL (' + sys.str(i) + '/30 rounds, ' +
              sys.str(fails) + ' errors)');
}
";

// Client coroutine -- runs the other side of the echo loop.
client_src = "sys = Import('sys');
tls = Import('tls');
sys.msleep(400);
conf = tls.conf_new('client', nil, nil, nil, nil, false);
fd = tls.connect('localhost', '18445', conf, 10000);
if (fd == false || fd == nil) {
    sys.print('handoff_client: connect failed');
    tls.conf_free(conf);
    return;
} fi
tls.set_sni(fd, 'localhost');
if (tls.handshake(fd, 10000) != true) {
    sys.print('handoff_client: handshake failed');
    tls.close(fd);
    tls.conf_free(conf);
    return;
} fi
i = 0;
while (i < 30) {
    msg = tls.recv(fd, 10000);
    if (msg == false || msg == nil || msg == true) break; fi
    if (tls.send(fd, msg, 10000) != true) break; fi
    i = i + 1;
}
tls.close(fd);
tls.conf_free(conf);
";

Eval(client_src, nil, true, 'handoff_client');

conf = tls.conf_new('server', CERT, KEY, nil, nil, false);
if (!conf) {
    sys.print('handoff_test: srv conf_new failed');
    return;
} fi

lfd = tls.listen(HOST, SERVICE);
if (lfd == false) {
    sys.print('handoff_test: srv listen failed');
    tls.conf_free(conf);
    return;
} fi

cfd = tls.accept(lfd, conf, TIMEOUT);
if (cfd == false || cfd == nil) {
    sys.print('handoff_test: srv accept failed');
    tls.close(lfd);
    tls.conf_free(conf);
    return;
} fi

// Hand the accepted TLS fd off to a worker coroutine.  The server
// coroutine then exits without calling tls.close(cfd) -- the worker
// owns it now.  The accept-side per-ctx chain must be empty here
// (the lt for cfd was never added to the server's chain after accept
// returned), so the server's ctx_tls_free is a no-op for cfd.  The
// lt stays alive in the per-lang rbtree for the worker.
Eval(worker_src, cfd, true, 'handoff_worker');

tls.close(lfd);
tls.conf_free(conf);
