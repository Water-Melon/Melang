// tls_test.m
//
// Self-contained, non-blocking TLS echo test.  Spawns its own server
// coroutine via Eval(..., true) (inline source) on the same Melang
// scheduler, connects from the main coroutine, and runs ROUNDS
// uniquely-numbered echo rounds.  No helper scripts; no external
// driver needed.
//
// Requirements:
//   - Melon built with --enable-tls and the host has OpenSSL.
//   - tls.so installed (or MELANG_DYNAMIC_PATH set).
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
// Run (from the tests/ directory):
//   ../bin/melang tls_test.m
//   ../bin/melang -t=2 tls_test.m       # multi-thread mode

sys = Import('sys');
str = Import('str');
tls = Import('tls');

HOST    = 'localhost';
SERVICE = '18443';
ROUNDS  = 100;
TIMEOUT = 10000;
CERT    = '/tmp/melang_tls_test.crt';
KEY     = '/tmp/melang_tls_test.key';

// Server coroutine -- inline source.  Listens on $SERVICE, accepts
// one TLS connection, echoes every payload back until ROUNDS
// completed or peer closes.  The outer file uses double-quoted
// strings so the inline body can keep single quotes for its own
// literals without escaping.
server_src = "sys = Import('sys');
tls = Import('tls');
conf = tls.conf_new('server',
                    '/tmp/melang_tls_test.crt',
                    '/tmp/melang_tls_test.key',
                    nil, nil, false);
if (!conf) {
    sys.print('srv: conf_new failed (check cert/key paths)');
    return;
} fi
lfd = tls.listen('localhost', '18443');
if (lfd == false) {
    sys.print('srv: listen failed');
    tls.conf_free(conf);
    return;
} fi
cfd = tls.accept(lfd, conf, 10000);
if (cfd == false || cfd == nil) {
    sys.print('srv: accept failed');
    tls.close(lfd);
    tls.conf_free(conf);
    return;
} fi
if (tls.handshake(cfd, 10000) != true) {
    sys.print('srv: handshake failed');
    tls.close(cfd);
    tls.close(lfd);
    tls.conf_free(conf);
    return;
} fi
i = 0;
while (i < 100) {
    msg = tls.recv(cfd, 10000);
    if (msg == false || msg == nil || msg == true) break; fi
    if (tls.send(cfd, msg, 10000) != true) break; fi
    i = i + 1;
}
tls.close(cfd);
tls.close(lfd);
tls.conf_free(conf);
";

Eval(server_src, nil, true, 'tls_test_server');
sys.msleep(300);

conf = tls.conf_new('client', nil, nil, nil, nil, false);
if (!conf) {
    sys.print('tls_test: client conf_new failed');
    return;
} fi

fd = tls.connect(HOST, SERVICE, conf, TIMEOUT);
if (fd == false || fd == nil) {
    sys.print('tls_test: client connect failed');
    tls.conf_free(conf);
    return;
} fi

tls.set_sni(fd, HOST);

ok = tls.handshake(fd, TIMEOUT);
if (ok != true) {
    sys.print('tls_test: client handshake failed');
    tls.close(fd);
    tls.conf_free(conf);
    return;
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
    // For small messages the server emits one TLS record per echo,
    // so a single recv usually suffices; loop for robustness in case
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
