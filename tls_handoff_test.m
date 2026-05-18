// tls_handoff_test.m
//
// Proves that the fd returned by tls.accept is fully portable: an
// accepting coroutine can hand the fd off to a fresh worker
// coroutine via Eval, and the worker can drive the full
// handshake / send / recv / close cycle.
//
// Topology:
//   - "server" coroutine (this file): accepts ONE TLS connection,
//     then spawns "worker" via Eval(...) passing the accepted fd
//     as EVAL_DATA.  The server then exits without touching the
//     fd again.  This exercises the cross-coroutine handoff plus
//     the cleanup path when the accepting coroutine terminates
//     while the fd is still in use.
//   - "client" coroutine (tls_handoff_client.m): connects and runs
//     the same N-round echo from the other side.
//   - "worker" coroutine (tls_handoff_worker.m): runs the echo on
//     the accepted fd.
//
// Requirements:
//   - Same as tls_test.m (Melon built with --enable-tls, cert/key
//     at /tmp/melang_tls_test.{crt,key}, port 18445 free).
//
// Run:
//   melang tls_handoff_test.m tls_handoff_client.m

sys = Import('sys');
tls = Import('tls');

HOST    = 'localhost';
SERVICE = '18445';
TIMEOUT = 10000;
CERT    = '/tmp/melang_tls_test.crt';
KEY     = '/tmp/melang_tls_test.key';

conf = tls.conf_new('server', CERT, KEY, nil, nil, false);
if (!conf) {
    sys.print('handoff_test: srv conf_new failed');
    sys.exit(1);
} fi

lfd = tls.listen(HOST, SERVICE);
if (lfd == false) {
    sys.print('handoff_test: srv listen failed');
    tls.conf_free(conf);
    sys.exit(1);
} fi

cfd = tls.accept(lfd, conf, TIMEOUT);
if (cfd == false || cfd == nil) {
    sys.print('handoff_test: srv accept failed');
    tls.close(lfd);
    tls.conf_free(conf);
    sys.exit(1);
} fi

// Hand the accepted TLS fd off to a worker coroutine.
Eval('tls_handoff_worker.m', cfd, false, 'handoff_worker');

// Server's job is done -- it does NOT call tls.close(cfd) (the
// worker owns it now) and exits.  The accept-side per-ctx chain
// must be empty at this point (the lt for cfd was never added to
// the server's chain after accept returned), so the server's
// ctx_tls_free should be a no-op for cfd.  The lt stays alive in
// the per-lang rbtree for the worker.
tls.close(lfd);
tls.conf_free(conf);
