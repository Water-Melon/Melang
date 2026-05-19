// tls_busy_server.m -- companion to tls_busy_test.m.
//
// Accepts a single TLS connection and waits ~1 second before echoing,
// so the client has a stable window in which to attempt a second
// concurrent operation on the same fd.

sys = Import('sys');
tls = Import('tls');

conf = tls.conf_new('server',
                    '/tmp/melang_tls_test.crt',
                    '/tmp/melang_tls_test.key',
                    nil, nil, false);
if (!conf) {
    sys.print('tls_busy_server: conf_new failed');
    sys.exit(1);
} fi

lfd = tls.listen('localhost', '18446');
if (lfd == false) {
    sys.print('tls_busy_server: listen failed');
    tls.conf_free(conf);
    sys.exit(1);
} fi

cfd = tls.accept(lfd, conf, 10000);
if (cfd == false || cfd == nil) {
    sys.print('tls_busy_server: accept failed');
    tls.close(lfd);
    tls.conf_free(conf);
    sys.exit(1);
} fi

if (tls.handshake(cfd, 10000) != true) {
    sys.print('tls_busy_server: handshake failed');
    tls.close(cfd);
    tls.close(lfd);
    tls.conf_free(conf);
    sys.exit(1);
} fi

msg = tls.recv(cfd, 10000);
sys.msleep(1500);
tls.send(cfd, msg, 10000);

tls.close(cfd);
tls.close(lfd);
tls.conf_free(conf);
