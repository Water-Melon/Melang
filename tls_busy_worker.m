// tls_busy_worker.m -- worker coroutine for tls_busy_test.m.
//
// Holds a recv in flight on the shared fd so the parent coroutine can
// observe that a second concurrent call returns false immediately.
// Closes the fd when its recv resolves.

sys = Import('sys');
tls = Import('tls');

fd = EVAL_DATA;
tls.recv(fd, 10000);
tls.close(fd);
