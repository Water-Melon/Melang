// https_test.m
//
// Self-contained end-to-end HTTPS exchange test.  Spawns its own
// server coroutine via Eval(..., true) (inline source) on the same
// scheduler, then calls http.https_request from the main coroutine
// for ROUNDS rounds and verifies each echo end-to-end.  No helper
// scripts.
//
// Requirements:
//   - Melon built with --enable-tls; Melang configure resolved [OpenSSL].
//   - tls.so and http.so installed (or MELANG_DYNAMIC_PATH set).
//   - Self-signed cert/key at /tmp/melang_tls_test.{crt,key} (see
//     tls_test.m header for the openssl invocation).
//   - Port 18444 free on localhost.
//
// Run (from the tests/ directory):
//   ../bin/melang https_test.m
//   ../bin/melang -t=2 https_test.m

sys  = Import('sys');
str  = Import('str');
http = Import('http');

HOST    = 'localhost';
SERVICE = '18444';
ROUNDS  = 50;
TIMEOUT = 15000;

// Inline server: accepts ROUNDS HTTPS connections in series,
// parses one HTTP/1.1 request per connection, answers a 200 that
// echoes the X-Iter header so the client can confirm round-trip.
server_src = "sys  = Import('sys');
tls  = Import('tls');
http = Import('http');
conf = tls.conf_new('server',
                    '/tmp/melang_tls_test.crt',
                    '/tmp/melang_tls_test.key',
                    nil, nil, false);
if (!conf) {
    sys.print('srv: conf_new failed (check cert/key paths)');
    return;
} fi
lfd = tls.listen('localhost', '18444');
if (lfd == false) {
    sys.print('srv: listen failed');
    tls.conf_free(conf);
    return;
} fi
served = 0;
while (served < 50) {
    cfd = tls.accept(lfd, conf, 15000);
    if (cfd == false || cfd == nil) {
        sys.print('srv: accept failed at ' + sys.str(served));
        break;
    } fi
    if (tls.handshake(cfd, 15000) != true) {
        sys.print('srv: handshake failed at ' + sys.str(served));
        tls.close(cfd);
        served = served + 1;
        continue;
    } fi
    buf = '';
    req = nil;
    while (req == nil) {
        chunk = tls.recv(cfd, 15000);
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
    if (tls.send(cfd, wire, 15000) != true) {
        sys.print('srv: send failed at ' + sys.str(served));
    } fi
    tls.close(cfd);
    served = served + 1;
}
tls.close(lfd);
tls.conf_free(conf);
";

Eval(server_src, nil, true, 'https_test_server');
sys.msleep(300);

fails = 0;
done  = 0;
i = 0;
while (i < ROUNDS) {
    iter_s = sys.str(i);
    args = ['host':    HOST,
            'service': SERVICE,
            'timeout': TIMEOUT,
            'verify':  false,
            'sni':     HOST,
            'request': ['type':    'request',
                        'method':  'GET',
                        'version': '1.1',
                        'uri':     '/echo',
                        'args':    '',
                        'headers': ['Host':           HOST,
                                    'User-Agent':     'melang-https-test',
                                    'X-Iter':         iter_s,
                                    'Content-Length': '0'],
                        'body':    '']];

    resp = http.https_request(args);
    if (resp == false || resp == nil) {
        sys.print('https_test: request ' + sys.str(i) + ' failed');
        fails = fails + 1;
        break;
    } fi
    if (resp['code'] != 200) {
        sys.print('https_test: request ' + sys.str(i) + ' code=' + sys.str(resp['code']));
        fails = fails + 1;
        break;
    } fi
    got = resp['headers']['X-Iter'];
    if (got == nil || !str.strcmp(got, iter_s)) {
        sys.print('https_test: request ' + sys.str(i) + ' X-Iter mismatch');
        fails = fails + 1;
        break;
    } fi
    sys.print('https_test: round ' + sys.str(i) + ' ok');
    i = i + 1;
    done = i;
}

if (fails == 0 && done == ROUNDS) {
    sys.print('https_test: PASS (' + sys.str(ROUNDS) + ' rounds)');
} else {
    sys.print('https_test: FAIL (' + sys.str(done) + '/' + sys.str(ROUNDS) +
              ' rounds, ' + sys.str(fails) + ' errors)');
}
