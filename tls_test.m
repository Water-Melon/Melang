// tls_test.m
//
// Multi-round non-blocking TLS echo test driven by two coroutines on
// one Melang event loop.  The client coroutine (this file) spawns a
// server coroutine via Eval('tls_server.m'); the server accepts a
// single TLS connection then echoes every payload back; the client
// sends ROUNDS uniquely-numbered payloads and verifies each echo.
//
// Requirements:
//   - Melon built with --enable-tls
//   - tls.so installed (or MELANG_DYNAMIC_PATH set)
//   - Self-signed cert/key at /tmp/melang_tls_test.{crt,key}:
//
//       openssl req -x509 -newkey rsa:2048 -nodes \
//         -keyout /tmp/melang_tls_test.key \
//         -out    /tmp/melang_tls_test.crt \
//         -subj   "/CN=localhost" \
//         -days   1
//
//   - Port 18443 free on localhost.
//
// Run:
//   melang tls_test.m

sys = Import('sys');
str = Import('str');
tls = Import('tls');

HOST    = 'localhost';
SERVICE = '18443';
ROUNDS  = 100;
TIMEOUT = 10000;

// Spawn the echo server as a sibling coroutine, then give it a moment
// to bind() before we connect.
Eval('tls_server.m', nil, false, 'tls_test_server');
sys.msleep(300);

conf = tls.conf_new('client', nil, nil, nil, nil, false);
if (!conf) {
    sys.print('tls_test: client conf_new failed');
    sys.exit(1);
} fi

fd = tls.connect(HOST, SERVICE, conf, TIMEOUT);
if (fd == false || fd == nil) {
    sys.print('tls_test: client connect failed');
    tls.conf_free(conf);
    sys.exit(1);
} fi

tls.set_sni(fd, HOST);

ok = tls.handshake(fd, TIMEOUT);
if (ok != true) {
    sys.print('tls_test: client handshake failed');
    tls.close(fd);
    tls.conf_free(conf);
    sys.exit(1);
} fi

fails = 0;
done  = 0;
i = 0;
while (i < ROUNDS) {
    out = 'iter=' + sys.str(i) + ';payload=ABCDEFGHIJKLMNOPQRSTUVWXYZ';
    if (tls.send(fd, out, TIMEOUT) != true) {
        sys.print('tls_test: client send failed at ' + sys.str(i));
        fails = fails + 1;
        break;
    } fi
    // Accumulate the echo until we have at least str.strlen(out) bytes.
    // For small messages the server emits one TLS record per echo so
    // typically a single recv suffices; we loop for robustness in case
    // the kernel splits the read across syscall boundaries.
    expected = str.strlen(out);
    got      = '';
    recv_failed = false;
    while (str.strlen(got) < expected) {
        chunk = tls.recv(fd, TIMEOUT);
        if (chunk == false || chunk == nil || chunk == true) {
            sys.print('tls_test: client recv failed at ' + sys.str(i));
            recv_failed = true;
            break;
        } fi
        got = got + chunk;
    }
    if (recv_failed) {
        fails = fails + 1;
        break;
    } fi
    // The server echoes one record per request -- so got should be
    // exactly str.strlen(out) bytes long.  A longer reply indicates
    // pipelining/corruption.
    if (str.strlen(got) != expected || !str.strcmp(got, out)) {
        sys.print('tls_test: mismatch at round ' + sys.str(i));
        fails = fails + 1;
        break;
    } fi
    sys.print('tls_test: round ' + sys.str(i) + ' ok');
    i = i + 1;
    done = i;
}

tls.close(fd);
tls.conf_free(conf);

if (fails == 0 && done == ROUNDS) {
    sys.print('tls_test: PASS (' + sys.str(ROUNDS) + ' rounds)');
} else {
    sys.print('tls_test: FAIL (' + sys.str(done) + '/' + sys.str(ROUNDS) +
              ' rounds, ' + sys.str(fails) + ' errors)');
}
