// tls_handoff_worker.m -- worker coroutine for tls_handoff_test.m
//
// Receives a TLS fd via EVAL_DATA, then runs the same N-round
// echo loop the test client would.  Exists to prove that an fd
// returned by tls.accept on one coroutine is fully usable from
// another coroutine on the same scheduler.

sys = Import('sys');
str = Import('str');
tls = Import('tls');

ROUNDS  = 30;
TIMEOUT = 10000;

fd = EVAL_DATA;

if (tls.handshake(fd, TIMEOUT) != true) {
    sys.print('worker: handshake failed');
    tls.close(fd);
    sys.exit(1);
} fi

i = 0;
fails = 0;
while (i < ROUNDS) {
    out = 'handoff=' + sys.str(i);
    if (tls.send(fd, out, TIMEOUT) != true) {
        sys.print('worker: send failed at ' + sys.str(i));
        fails = fails + 1;
        break;
    } fi
    expected = str.strlen(out);
    got = '';
    recv_failed = false;
    while (str.strlen(got) < expected) {
        chunk = tls.recv(fd, TIMEOUT);
        if (chunk == false || chunk == nil || chunk == true) {
            sys.print('worker: recv failed at ' + sys.str(i));
            recv_failed = true;
            break;
        } fi
        got = got + chunk;
    }
    if (recv_failed) { fails = fails + 1; break; } fi
    if (str.strlen(got) != expected || !str.strcmp(got, out)) {
        sys.print('worker: mismatch at ' + sys.str(i));
        fails = fails + 1;
        break;
    } fi
    sys.print('handoff_test: round ' + sys.str(i) + ' ok');
    i = i + 1;
}

tls.close(fd);

if (fails == 0 && i == ROUNDS) {
    sys.print('handoff_test: PASS (' + sys.str(ROUNDS) + ' rounds)');
} else {
    sys.print('handoff_test: FAIL (' + sys.str(i) + '/' + sys.str(ROUNDS) +
              ' rounds, ' + sys.str(fails) + ' errors)');
}
