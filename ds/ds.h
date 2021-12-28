#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define ANN       __attribute__((nonnull))

typedef uintptr_t reg_t;

typedef enum ds_opcode {
  dsop_imm,

  dsop_add_imm, dsop_sub_imm, dsop_mul_imm,
  dsop_div_imm, dsop_mod_imm,

  dsop_add, dsop_sub, dsop_mul,
  dsop_div, dsop_mod,

  dsop_eq, dsop_ne, dsop_land, dsop_lor,
  dsop_gt, dsop_ge, dsop_lt, dsop_le,

  dsop_band,dsop_bor, dsop_bxor,
  dsop_blshift, dsop_brshift,

  dsop_inc, dsop_dec,
  dsop_neg, dsop_not, dsop_cmp,

  dsop_jump,
  dsop_eq_jump, dsop_ne_jump,
  dsop_gt_jump, dsop_ge_jump,
  dsop_lt_jump, dsop_le_jump,

  dsop_call, dsop_return,
  dsop_end,
  dsop_max,
} ds_opcode;

typedef struct {
  reg_t *code;
  reg_t  *reg;
} Frame;

//ANN 

__attribute__((nonnull(1)))
void dsvm_run(reg_t *, const reg_t *);

reg_t *code_alloc(const ds_opcode, const uint32_t);

static inline reg_t *dscode_imm(const reg_t imm, const reg_t dest) {
  reg_t *const code = code_alloc(dsop_imm, 2);
  code[1] = imm;
  code[2] = dest;
  return code;
}

ANN static inline reg_t *dscode_call(const reg_t *func, const reg_t offset) {
  reg_t *const code = code_alloc(dsop_call, 2);
  code[1] = (reg_t)func;
  code[2] = offset;
  return code;
}

static inline reg_t *dscode_return(void) {
  return code_alloc(dsop_return, 0);
}

static inline reg_t *dscode_end(void) {
  return code_alloc(dsop_end, 0);
}

static inline reg_t *dscode_binary(const ds_opcode op, const reg_t src, const reg_t imm, const reg_t dest) {\
  reg_t *const code = code_alloc(op, 3);\
  code[1] = src;\
  code[2] = imm;\
  code[3] = dest;\
  return code;\
}

static inline reg_t *dscode_ibinary(const ds_opcode op, const reg_t src, const reg_t imm, const reg_t dest) {\
  reg_t *const code = code_alloc(op - 5, 3);\
  code[1] = src;\
  code[2] = imm;\
  code[3] = dest;\
  return code;\
}

static inline reg_t *dscode_unary(const ds_opcode op, const reg_t src, const reg_t dest) {
  reg_t *const code = code_alloc(op, 2);
  code[1] = src;
  code[2] = dest;
  return code;
}

ANN static inline reg_t *dscode_jump_op(const ds_opcode op, const reg_t src, const reg_t imm, const reg_t *new_code) {
  if(op == dsop_jump) {
    reg_t *const code = code_alloc(dsop_jump, 1);
    code[1] = (reg_t)new_code;
    return code;
  }
  reg_t *const code = code_alloc(op, 3);
  code[1] = src;
  code[2] = imm;
  code[3] = (reg_t)new_code;
  return code;
}

reg_t *dscode_start(void);
