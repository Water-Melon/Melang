// tls_handoff_client.m -- client coroutine for tls_handoff_test.m
//
// Connects to the server, drives the same ROUNDS-round echo as
// the worker on the server side.  Loops on recv until the
// payload is fully echoed back.

sys = Import('sys');
str = Import('str');
tls = Import('tls');

HOST    = 'localhost';
SERVICE = '18445';
ROUNDS  = 30;
TIMEOUT = 10000;

// Give the server time to bind() before connecting.
sys.msleep(400);

conf = tls.conf_new('client', nil, nil, nil, nil, false);
fd   = tls.connect(HOST, SERVICE, conf, TIMEOUT);
if (fd == false || fd == nil) {
    sys.print('handoff_client: connect failed');
    tls.conf_free(conf);
    sys.exit(1);
} fi
tls.set_sni(fd, HOST);

if (tls.handshake(fd, TIMEOUT) != true) {
    sys.print('handoff_client: handshake failed');
    tls.close(fd);
    tls.conf_free(conf);
    sys.exit(1);
} fi

i = 0;
while (i < ROUNDS) {
    msg = tls.recv(fd, TIMEOUT);
    if (msg == false || msg == nil || msg == true) {
        sys.print('handoff_client: recv break at ' + sys.str(i));
        break;
    } fi
    if (tls.send(fd, msg, TIMEOUT) != true) {
        sys.print('handoff_client: send break at ' + sys.str(i));
        break;
    } fi
    i = i + 1;
}

tls.close(fd);
tls.conf_free(conf);
