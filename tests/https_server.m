// https_server.m -- helper coroutine for https_test.m
//
// Spawned via Eval('https_server.m', nil, false, 'https_server').
// Accepts ROUNDS connections in series, expecting one HTTP/1.1
// request per connection.  For each request it echoes the X-Iter
// header in a 200 response.
//
// Keep settings in sync with https_test.m -- Eval can only carry a
// single scalar value.

sys  = Import('sys');
tls  = Import('tls');
http = Import('http');

HOST    = 'localhost';
SERVICE = '18444';
ROUNDS  = 50;
TIMEOUT = 15000;
CERT    = '/tmp/melang_tls_test.crt';
KEY     = '/tmp/melang_tls_test.key';

conf = tls.conf_new('server', CERT, KEY, nil, nil, false);
if (!conf) {
    sys.print('srv: conf_new failed');
    sys.exit(1);
} fi

lfd = tls.listen(HOST, SERVICE);
if (lfd == false) {
    sys.print('srv: listen failed');
    tls.conf_free(conf);
    sys.exit(1);
} fi

served = 0;
while (served < ROUNDS) {
    cfd = tls.accept(lfd, conf, TIMEOUT);
    if (cfd == false || cfd == nil) {
        sys.print('srv: accept failed at ' + sys.str(served));
        break;
    } fi
    // Drive the handshake so a TLS-level failure doesn't masquerade
    // as a parse error on the first recv.
    if (tls.handshake(cfd, TIMEOUT) != true) {
        sys.print('srv: handshake failed at ' + sys.str(served));
        tls.close(cfd);
        served = served + 1;
        continue;
    } fi

    buf = '';
    req = nil;
    while (req == nil) {
        chunk = tls.recv(cfd, TIMEOUT);
        if (chunk == false || chunk == nil || chunk == true) {
            sys.print('srv: recv failed at ' + sys.str(served));
            break;
        } fi
        buf = buf + chunk;
        req = http.parse(buf);
    }
    if (req == nil || req == false) {
        tls.close(cfd);
        served = served + 1;
        continue;
    } fi

    iter_val = req['headers']['X-Iter'];
    if (iter_val == nil) iter_val = '?'; fi

    resp = ['type':    'response',
            'version': '1.1',
            'code':    200,
            'headers': ['Server':         'melang-https-test',
                        'X-Iter':         iter_val,
                        'Content-Length': '0'],
            'body':    ''];

    wire = http.create(resp);
    if (tls.send(cfd, wire, TIMEOUT) != true) {
        sys.print('srv: send failed at ' + sys.str(served));
    } fi

    tls.close(cfd);
    served = served + 1;
}

tls.close(lfd);
tls.conf_free(conf);
