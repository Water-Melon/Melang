/*
 * matrix_overflow_test.m — regression test for the matrix builder
 * integer-overflow heap overflow.
 *
 * Background: mln_lang_array2matrix() reads the script-supplied 'row' and
 * 'col' as mln_size_t and only checks that the 'data' element count equals
 * row*col. It never guarded the multiplication against overflow. A script
 * can therefore pass dimensions whose product wraps mln_size_t to 0 (matching
 * an empty 'data' array), producing an mln_matrix_t with enormous recorded
 * row/col but a zero-sized buffer. mln_matrix_mul() then allocates a buffer
 * from the (also overflowed) product while iterating the *true* huge
 * dimensions, reading and writing far out of bounds -> heap smash / SIGSEGV.
 *
 * 2^33 * 2^31 = 2^64 == 0 (mod 2^64); col(2^31) == the peer's row(2^31), so the
 * m1->col == m2->row gate in mln_matrix_mul is satisfied and the overflowed
 * multiply path is reached.
 *
 * The call is wrapped in an isolated Eval() coroutine so the *fixed* build's
 * graceful rejection ("Invalid argument.") only aborts that coroutine, letting
 * the driver finish and print the OK banner.
 *
 * Expected behaviour:
 *   - Fixed build : the overflowing dimensions are rejected; the process
 *                   finishes normally and exits 0.
 *   - Vulnerable build: mln_matrix_mul overruns its buffers and the process is
 *                   aborted (SIGSEGV / SIGABRT), failing the test.
 */
sys = Import('sys');

Eval("m = Import('matrix'); " +
     "a1 = ['row': 8589934592, 'col': 2147483648, 'data': []]; " +
     "a2 = ['row': 2147483648, 'col': 8589934592, 'data': []]; " +
     "m.mul(a1, a2);", nil, true, 'mat_overflow');

sys.print('matrix_overflow_test: OK (overflowed dimensions handled without heap overflow)');
