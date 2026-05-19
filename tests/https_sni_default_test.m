// https_sni_default_test.m
//
// Verifies that http.https_request defaults `sni` to `host` when the
// caller omits it.  The test server requires SNI ("localhost") to
// complete the handshake; if our default doesn't fire, the handshake
// fails and https_request returns false.
//
// Run (from the tests/ directory):
//   ../bin/melang https_sni_default_test.m

sys  = Import('sys');
http = Import('http');

Eval('https_sni_default_server.m', nil, false, 'sni_default_server');
sys.msleep(300);

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
    sys.exit(1);
} fi

sys.print('https_sni_default_test: PASS (default SNI applied)');
