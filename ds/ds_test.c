#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "ds.h"

static reg_t *make_main(const char *arg, const reg_t *fib) {
  reg_t *const code = dscode_imm(atoi(arg), 0);
  (void)dscode_call(fib, 0);
  (void)dscode_end();
  return code;
}

static reg_t *make_fib(void) {
  reg_t *const code = dscode_imm(2, 1);
  (void)dscode_lt_jump(0, 1, 8);
  (void)dscode_return();
  (void)dscode_sub_imm(0, 2, 1);
  (void)dscode_call(code, 1);
  (void)dscode_sub_imm(0, 1, 3);
  (void)dscode_call(code, 3);
  (void)dscode_add(1, 3, 0);
  (void)dscode_return();
  return code;
}

int main(int argc, char **argv) {
  if(argc < 2) {
    puts("usage: ds_test <number>");
    return EXIT_FAILURE;
  }
  reg_t *const fib = make_fib(),
        *const main = make_main(argv[1], fib);
  dsvm_run(main);
  return EXIT_SUCCESS;
}
