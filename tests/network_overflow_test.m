/*
 * network_overflow_test.m — regression test for the udp_send 'service'
 * stack buffer overflow.
 *
 * Background: mln_lang_network_udp_send_process() copies the script-supplied
 * 'service' argument into a 64-byte stack buffer (`char service[64]`), but the
 * length guard mistakenly compared against sizeof(host)-1 (127). A script
 * passing a 'service' string of 64..127 bytes therefore overflowed the stack
 * buffer by up to 63 bytes — a script-triggerable stack smash.
 *
 * The dangerous memcpy runs before the udp fd is validated, so an invalid fd
 * is enough to reach it. We pass a 100-byte service string.
 *
 * The call is wrapped in an isolated Eval() coroutine so that the *fixed*
 * build's graceful rejection ("Invalid service.") only aborts that coroutine,
 * letting the driver finish and print the OK banner.
 *
 * Expected behaviour:
 *   - Fixed build : the oversized 'service' is rejected; the process finishes
 *                   normally and exits 0.
 *   - Vulnerable build: the memcpy overruns service[64] and the process is
 *                   aborted (e.g. "*** buffer overflow detected ***",
 *                   SIGABRT / exit 134), failing the test.
 */
sys = Import('sys');

/* Build a 100-byte service string (> sizeof(service)-1 == 63, but
 * <= sizeof(host)-1 == 127, i.e. exactly the vulnerable range). */
svc = '';
i = 0;
while (i < 100) { svc = svc + 'a'; i = i + 1; }

/* Reach the vulnerable memcpy with an invalid fd. On a fixed build this
 * raises "Invalid service." inside the spawned coroutine and does NOT crash. */
Eval("net = Import('net'); net.udp_send(0, 'x', '127.0.0.1', EVAL_DATA);", svc, true, 'udp_overflow');

sys.print('network_overflow_test: OK (oversized service handled without stack overflow)');
