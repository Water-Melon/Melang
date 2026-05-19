// tls_server.m -- helper coroutine for tls_test.m
//
// Spawned via Eval('tls_server.m', nil, false, 'tls_server').  Listens
// on TLS, accepts one connection, then echoes every received message
// back to the client until ROUNDS rounds are complete or either side
// closes.
//
// Note: settings are kept in sync with tls_test.m by hand (Eval
// transports a single scalar, not a dict).

sys = Import('sys');
tls = Import('tls');

HOST    = 'localhost';
SERVICE = '18443';
ROUNDS  = 100;
TIMEOUT = 10000;
CERT    = '/tmp/melang_tls_test.crt';
KEY     = '/tmp/melang_tls_test.key';

conf = tls.conf_new('server', CERT, KEY, nil, nil, false);
if (!conf) {
    sys.print('srv: conf_new failed (check cert/key paths)');
    sys.exit(1);
} fi

lfd = tls.listen(HOST, SERVICE);
if (lfd == false) {
    sys.print('srv: listen failed');
    tls.conf_free(conf);
    sys.exit(1);
} fi

cfd = tls.accept(lfd, conf, TIMEOUT);
if (cfd == false || cfd == nil) {
    sys.print('srv: accept failed');
    tls.close(lfd);
    tls.conf_free(conf);
    sys.exit(1);
} fi

ok = tls.handshake(cfd, TIMEOUT);
if (ok != true) {
    sys.print('srv: handshake failed');
    tls.close(cfd);
    tls.close(lfd);
    tls.conf_free(conf);
    sys.exit(1);
} fi

i = 0;
while (i < ROUNDS) {
    msg = tls.recv(cfd, TIMEOUT);
    if (msg == false || msg == nil || msg == true) {
        sys.print('srv: recv break at ' + sys.str(i));
        break;
    } fi
    if (tls.send(cfd, msg, TIMEOUT) != true) {
        sys.print('srv: send break at ' + sys.str(i));
        break;
    } fi
    i = i + 1;
}

tls.close(cfd);
tls.close(lfd);
tls.conf_free(conf);
