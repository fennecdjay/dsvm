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
  dscode_if_op(dsop_if_lt_imm, 0, 2);
  (void)dscode_return(0);

//  dscode_set_else(code, dscode_start());
  dscode_if_dest(code, dscode_start());
  (void)dscode_ibinary(dsop_sub_imm, 0, 1, 2);
  (void)dscode_call(code, 2, 3);

  (void)dscode_ibinary(dsop_sub_imm, 0, 2, 4);
  (void)dscode_call(code, 4, 4);

  (void)dscode_binary(dsop_add, 3, 5, 6);
  (void)dscode_return(6);

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
  DsFrame frames[256];

  DsThread thread = { .code = main, .reg = reg, .frames = frames };
  dsvm_run(&thread, 0);
  printf("result: %lu\n", reg[frames->out]);
  return EXIT_SUCCESS;
}
