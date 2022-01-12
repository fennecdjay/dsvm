#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "ds.h"

static dscode_t *make_main(const char *arg, const dscode_t *fib) {
  dscode_t *const code = dscode_start();
  (void)dscode_imm(atoi(arg), 0);
  (void)dscode_call(fib, 0, 0);
  (void)dscode_end();
  DsThread thread = { .code = code };
  dsvm_run(&thread, dscode_start());
  return code;
}

static dscode_t *make_fib(void) {
  dscode_t *const code = dscode_start();
  (void)dscode_imm(2, 1);
  (void)dscode_binary(dsop_lt, 0, 1, 2);

  dscode_t *const if_code = dscode_start();
  (void)dscode_if(2);

  dscode_set_if(if_code, dscode_start());
  (void)dscode_return(0);

  dscode_set_else(if_code, dscode_start());
  (void)dscode_imm(1, 3);
  (void)dscode_binary(dsop_sub, 0, 3, 4);
  (void)dscode_call(code, 4, 5);

  (void)dscode_imm(2, 6);
  (void)dscode_binary(dsop_sub, 0, 6, 7);
  (void)dscode_call(code, 7, 8);

  (void)dscode_binary(dsop_add, 5, 8, 9);
  (void)dscode_return(9);

  DsThread thread = { .code = code };
  dsvm_run(&thread, dscode_start());
  return code;
}

int main(int argc, char **argv) {
  if(argc < 2) {
    puts("usage: ds_test <number>");
    return EXIT_FAILURE;
  }
  dscode_t *const fib = make_fib(),
       *const main = make_main(argv[1], fib);
  reg_t reg[256];
  Frame frames[256];

  DsThread thread = { .code = main, .reg = reg, .frames = frames };
  dsvm_run(&thread, 0);
  printf("result: %lu\n", reg[frames->out]);
  return EXIT_SUCCESS;
}
