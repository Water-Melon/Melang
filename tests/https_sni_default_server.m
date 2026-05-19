// https_sni_default_server.m -- single-shot HTTPS server for the
// SNI-default test.  Listens on port 18447, accepts one connection,
// answers a single GET, then exits.

sys  = Import('sys');
tls  = Import('tls');
http = Import('http');

CERT    = '/tmp/melang_tls_test.crt';
KEY     = '/tmp/melang_tls_test.key';
TIMEOUT = 10000;

conf = tls.conf_new('server', CERT, KEY, nil, nil, false);
if (!conf) {
    sys.print('sni_default_server: conf_new failed');
    sys.exit(1);
} fi

lfd = tls.listen('localhost', '18447');
if (lfd == false) {
    sys.print('sni_default_server: listen failed');
    tls.conf_free(conf);
    sys.exit(1);
} fi

cfd = tls.accept(lfd, conf, TIMEOUT);
if (cfd == false || cfd == nil) {
    sys.print('sni_default_server: accept failed');
    tls.close(lfd);
    tls.conf_free(conf);
    sys.exit(1);
} fi

if (tls.handshake(cfd, TIMEOUT) != true) {
    sys.print('sni_default_server: handshake failed (no SNI sent?)');
    tls.close(cfd);
    tls.close(lfd);
    tls.conf_free(conf);
    sys.exit(1);
} fi

buf = '';
req = nil;
while (req == nil) {
    chunk = tls.recv(cfd, TIMEOUT);
    if (chunk == false || chunk == nil || chunk == true) break; fi
    buf = buf + chunk;
    req = http.parse(buf);
}

resp = ['type':    'response',
        'version': '1.1',
        'code':    200,
        'headers': ['Server':         'sni-default-test',
                    'Content-Length': '0'],
        'body':    ''];
wire = http.create(resp);
tls.send(cfd, wire, TIMEOUT);

tls.close(cfd);
tls.close(lfd);
tls.conf_free(conf);
