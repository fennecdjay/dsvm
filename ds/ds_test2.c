#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "ds.h"

static dscode_t *make_bu(void) {
  char *const code = dscode_call2(calloc, 3, sizeof(reg_t), 2);
//  dscode_t *const code = dscode_imm(0, 3);
//  (void)dscode_jump_ne(dsop_eq_jump, 0, 1, code + 21); // addr !!!
//  (void)dscode_binary(dsop_add, $0, $0, $4);
// set left
// set right
//  (void)dscode_return(0);
// new object
// set item
// return object
  (void)dscode_return(2);
  dsvm_run(code, dscode_start());
  return code;
}

static dscode_t *make_item_check(void) {
// if tree.left
}
static dscode_t *make_main(const char *arg, const dscode_t *bu) {
  dscode_t *const code = dscode_imm(0, 0);
  (void)dscode_imm(1, 1);
  (void)dscode_call(bu, 0, 0);
  (void)dscode_end();
  dsvm_run(code, dscode_start());
  return code;
}

int main(int argc, char **argv) {
  dscode_t *const bu = make_bu(),
       *const main = make_main(argv[1], bu);
  dsvm_run(main, 0);
  return EXIT_SUCCESS;
}
