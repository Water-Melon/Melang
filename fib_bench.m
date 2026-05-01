/* Benchmark script: recursive Fibonacci(30) -> 832040 */
@F(i)
{
  if (i <= 2) {return 1;} fi
  return F(i - 1) + F(i - 2);
}

F(30);
