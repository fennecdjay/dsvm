#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef uintptr_t reg_t;

typedef enum ds_opcode {
  dsop_imm,

  dsop_add_imm, dsop_sub_imm, dsop_mul_imm,
  dsop_div_imm, dsop_mod_imm,

  dsop_add, dsop_sub, dsop_mul,
  dsop_div, dsop_mod,

  dsop_es, dsop_ne,
  dsop_gt, dsop_ge, dsop_lt, dsop_le,

  dsop_land, dsop_lor,

  dsop_band,dsop_bor, dsop_bxor,
  dsop_blshift, dsop_brshift,

  dsop_inc, dsop_dec,
  dsop_neg, dsop_not, dsop_cmp,

  dsop_call, dsop_return,

  dsop_jump,
  dsop_eq_branch, dsop_ne_branch,
  dsop_gt_branch, dsop_ge_branch,
  dsop_lt_branch, dsop_le_branch,

  dsop_end,
  dsop_max,
} ds_opcode;

typedef struct RvmInstr {
  reg_t op;
  reg_t lhs;
  reg_t rhs;
  reg_t ret;
} RvmInstr;

typedef struct {
  uintptr_t *data;
  uint32_t  offset;
  uint32_t  pc;
} Frame;

void run(reg_t *data);
