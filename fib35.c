#include "stdio.h"
long fib(long n) {
  if(n < 2) return n;
  return fib(n-1) + fib(n-2);
}

int main() {
  printf("result :%lu\n", fib(35));
}
