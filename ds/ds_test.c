#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "ds.h"

static dscode_t *make_main(const char *arg, const dscode_t *fib) {
  dscode_t *const code = dscode_imm(atoi(arg), 0);
  (void)dscode_call(fib, 0, 0);
  (void)dscode_end();
  dsvm_run(code, dscode_start());
  return code;
}

static dscode_t *make_fib(void) {
  dscode_t *const code = dscode_imm(2, 1);
  (void)dscode_jump_op(dsop_lt_jump, 0, 1, code + 21);
  (void)dscode_ibinary(dsop_sub, 0, 2, 1);
  (void)dscode_call(code, 1, 1);
  (void)dscode_ibinary(dsop_sub, 0, 1, 3);
  (void)dscode_call(code, 3, 3);
  (void)dscode_binary(dsop_add, 1, 3, 0);
  (void)dscode_return(0);
  dsvm_run(code, dscode_start());
  return code;
}

int main(int argc, char **argv) {
  if(argc < 2) {
    puts("usage: ds_test <number>");
    return EXIT_FAILURE;
  }
  dscode_t *const fib = make_fib(),
       *const main = make_main(argv[1], fib);
  dsvm_run(main, 0);
  return EXIT_SUCCESS;
}
