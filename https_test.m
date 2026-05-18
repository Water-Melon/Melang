// https_test.m
//
// End-to-end HTTPS exchange test driven by two coroutines on one
// event loop.  The server coroutine (https_server.m) accepts ROUNDS
// connections, parses one HTTP/1.1 request per connection, and
// answers with a 200 that echoes the X-Iter request header.  The
// client coroutine (this file) calls http.https_request ROUNDS times
// and verifies each echo end-to-end.
//
// Requirements:
//   - Melon built with --enable-tls
//   - tls.so and http.so installed (or MELANG_DYNAMIC_PATH set)
//   - Self-signed cert/key at /tmp/melang_tls_test.{crt,key}
//   - Port 18444 free on localhost.
//
// Run:
//   melang https_test.m

sys  = Import('sys');
str  = Import('str');
http = Import('http');

HOST    = 'localhost';
SERVICE = '18444';
ROUNDS  = 50;
TIMEOUT = 15000;

Eval('https_server.m', nil, false, 'https_test_server');
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
