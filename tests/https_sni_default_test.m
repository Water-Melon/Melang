// https_sni_default_test.m
//
// Self-contained.  Verifies that http.https_request defaults `sni`
// to `host` when the caller omits it.  The inline test server
// requires SNI ("localhost") to complete the handshake; if the
// default doesn't fire, the handshake fails and https_request
// returns false.
//
// Run (from the tests/ directory):
//   ../bin/melang https_sni_default_test.m
//   ../bin/melang -t=2 https_sni_default_test.m

sys  = Import('sys');
http = Import('http');

// Inline single-shot HTTPS server on 18447.  Accepts one
// connection, drives the handshake (which will fail without SNI),
// answers one GET, then exits.
server_src = "sys  = Import('sys');
tls  = Import('tls');
http = Import('http');
conf = tls.conf_new('server',
                    '/tmp/melang_tls_test.crt',
                    '/tmp/melang_tls_test.key',
                    nil, nil, false);
if (!conf) {
    sys.print('sni_default_server: conf_new failed');
    return;
} fi
lfd = tls.listen('localhost', '18447');
if (lfd == false) {
    sys.print('sni_default_server: listen failed');
    tls.conf_free(conf);
    return;
} fi
cfd = tls.accept(lfd, conf, 10000);
if (cfd == false || cfd == nil) {
    sys.print('sni_default_server: accept failed');
    tls.close(lfd);
    tls.conf_free(conf);
    return;
} fi
if (tls.handshake(cfd, 10000) != true) {
    sys.print('sni_default_server: handshake failed (no SNI sent?)');
    tls.close(cfd);
    tls.close(lfd);
    tls.conf_free(conf);
    return;
} fi
buf = '';
req = nil;
while (req == nil) {
    chunk = tls.recv(cfd, 10000);
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
tls.send(cfd, wire, 10000);
tls.close(cfd);
tls.close(lfd);
tls.conf_free(conf);
";

Eval(server_src, nil, true, 'sni_default_server');
sys.msleep(300);

// NOTE: deliberately no 'sni' key here -- we want the runtime
// default-to-host to fire.
args = ['host':    'localhost',
        'service': '18447',
        'verify':  false,
        'timeout': 10000,
        'request': ['type':    'request',
                    'method':  'GET',
                    'version': '1.1',
                    'uri':     '/',
                    'args':    '',
                    'headers': ['Host':           'localhost',
                                'User-Agent':     'sni-test',
                                'Content-Length': '0'],
                    'body':    '']];

resp = http.https_request(args);

if (resp == false || resp == nil) {
    sys.print('https_sni_default_test: FAIL (request failed; default SNI not applied)');
    return;
} fi

sys.print('https_sni_default_test: PASS (default SNI applied)');
