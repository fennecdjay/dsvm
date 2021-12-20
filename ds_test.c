#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "ds.h"

static reg_t *make_main(const char *arg, const reg_t *fib) {
  reg_t *code = malloc(7*sizeof(reg_t));

  code[0] = dsop_imm;
  code[1] = atoi(arg);
  code[2] = 0;

  code[3] = dsop_call;
  code[4] = (uintptr_t)fib;
  code[5] = 0;

  code[6] = dsop_end;
  return code;
}

static reg_t *make_fib(void) {
  reg_t *code = malloc(27 * sizeof(reg_t));

  code[0] = dsop_imm;
  code[1] = 2;
  code[2] = 1;

  code[3] = dsop_lt_branch;
  code[4] = 0;
  code[5] = 1;
  code[6] = 8;

  code[7] = dsop_return;

  code[8] = dsop_sub_imm;
  code[9] = 0;
  code[10] = 2;
  code[11] = 1;

  code[12] = dsop_call;
  code[13] = (uintptr_t)code;
  code[14] = 1;

  code[15] = dsop_sub_imm;
  code[16] = 0;
  code[17] = 1;
  code[18] = 3;

  code[19] = dsop_call;
  code[20] = (uintptr_t)code;
  code[21] = 3;

  code[22] = dsop_add;
  code[23] = 1;
  code[24] = 3;
  code[25] = 0;

  code[26] = dsop_return;
  return code;
}

int main(int argc, char **argv) {
  if(argc < 2) {
    puts("usage: ds_test <number>");
    return EXIT_FAILURE;
  }
  reg_t *const fib = make_fib(),
        *const main = make_main(argv[1], fib);
  run(main);
  free(fib);
  free(main);
  return EXIT_SUCCESS;
}
