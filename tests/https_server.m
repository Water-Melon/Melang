// https_server.m -- standalone long-running HTTPS example server.
//
// Listens on https://localhost:18444 and answers every request with
// a 200 OK that echoes a few request headers.  Runs until killed.
// Designed to be driven by curl / ab / wrk, not by another Melang
// script (the self-contained tests use their own inline servers).
//
// Requirements:
//   - Melon built with --enable-tls; Melang configure resolved [OpenSSL].
//   - tls.so and http.so installed (or MELANG_DYNAMIC_PATH set).
//   - Self-signed cert/key at /tmp/melang_tls_test.{crt,key}.  Generate
//     one with an SAN list that includes both 'localhost' and IP
//     127.0.0.1 / ::1 so curl can verify the cert when hitting either
//     name:
//
//       openssl req -x509 -newkey rsa:2048 -nodes \
//         -keyout /tmp/melang_tls_test.key \
//         -out    /tmp/melang_tls_test.crt \
//         -subj   "/CN=localhost" \
//         -addext "subjectAltName=DNS:localhost,IP:127.0.0.1,IP:::1" \
//         -days   1
//
//   - Port 18444 free on localhost.
//
// Run:
//   ../bin/melang https_server.m
//
// Drive with curl (cert is self-signed, so pass either --cacert or -k):
//   curl --cacert /tmp/melang_tls_test.crt https://localhost:18444/
//   curl --cacert /tmp/melang_tls_test.crt https://127.0.0.1:18444/
//   curl -k https://127.0.0.1:18444/                       # skip verify
//
// Drive with ab (Apache Bench):
//   ab -n 1000 -c 16 https://localhost:18444/
//
// Each accepted connection is handed off to a per-connection worker
// coroutine; the main loop returns to accept() immediately, so the
// server happily multiplexes many concurrent clients on a single
// event loop.

sys  = Import('sys');
tls  = Import('tls');
http = Import('http');

HOST    = 'localhost';
SERVICE = '18444';
CERT    = '/tmp/melang_tls_test.crt';
KEY     = '/tmp/melang_tls_test.key';
TIMEOUT = 30000;

// Per-connection worker -- handshakes, parses one request, replies,
// closes.  Receives the accepted fd via EVAL_DATA.
worker_src = "sys  = Import('sys');
str  = Import('str');
tls  = Import('tls');
http = Import('http');
fd = EVAL_DATA;
if (tls.handshake(fd, 30000) != true) {
    tls.close(fd);
    return;
} fi
buf = '';
req = nil;
while (req == nil) {
    chunk = tls.recv(fd, 30000);
    if (chunk == false || chunk == nil || chunk == true) {
        tls.close(fd);
        return;
    } fi
    buf = buf + chunk;
    req = http.parse(buf);
}
if (req == nil || req == false) {
    tls.close(fd);
    return;
} fi
ua    = req['headers']['User-Agent'];   if (ua == nil)    ua    = '-'; fi
uri   = req['uri'];                     if (uri == nil)   uri   = '-'; fi
meth  = req['method'];                  if (meth == nil)  meth  = '-'; fi
sys.print('https_server: ' + meth + ' ' + uri + ' ua=' + ua);
body = 'hello from melang https_server\n' +
       'method: ' + meth + '\n' +
       'uri:    ' + uri + '\n' +
       'ua:     ' + ua + '\n';
resp = ['type':    'response',
        'version': '1.1',
        'code':    200,
        'headers': ['Server':         'melang-https-server',
                    'Content-Type':   'text/plain; charset=utf-8',
                    'Content-Length': sys.str(str.strlen(body)),
                    'Connection':     'close'],
        'body':    body];
wire = http.create(resp);
tls.send(fd, wire, 30000);
tls.close(fd);
";

conf = tls.conf_new('server', CERT, KEY, nil, nil, false);
if (!conf) {
    sys.print('https_server: conf_new failed (check cert/key at /tmp/melang_tls_test.{crt,key})');
    return;
} fi

lfd = tls.listen(HOST, SERVICE);
if (lfd == false) {
    sys.print('https_server: listen failed (port ' + SERVICE + ' in use?)');
    tls.conf_free(conf);
    return;
} fi

sys.print('https_server: listening on https://' + HOST + ':' + SERVICE + '/');
sys.print('https_server: drive with  curl --cacert ' + CERT + ' https://localhost:' + SERVICE + '/');

n = 0;
while (true) {
    cfd = tls.accept(lfd, conf, TIMEOUT);
    if (cfd == false || cfd == nil) {
        // accept timeout (waiting for the next client) -- just loop.
        continue;
    } fi
    Eval(worker_src, cfd, true, 'https_server_worker');
    n = n + 1;
}

// Unreachable, but keep the cleanup so the intent is clear.
tls.close(lfd);
tls.conf_free(conf);
