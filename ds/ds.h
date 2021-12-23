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

  dsop_eq, dsop_ne, dsop_land, dsop_lor,
  dsop_gt, dsop_ge, dsop_lt, dsop_le,


  dsop_band,dsop_bor, dsop_bxor,
  dsop_blshift, dsop_brshift,

  dsop_inc, dsop_dec,
  dsop_neg, dsop_not, dsop_cmp,

  dsop_call, dsop_return,

  dsop_jump,
  dsop_eq_jump, dsop_ne_jump,
  dsop_gt_jump, dsop_ge_jump,
  dsop_lt_jump, dsop_le_jump,

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

void dsvm_run(reg_t *);

reg_t *code_alloc(const ds_opcode, const unsigned int);

static inline reg_t *dscode_imm(const reg_t imm, const reg_t dest) {
  reg_t *const code = code_alloc(dsop_imm, 2);
  code[1] = imm;
  code[2] = dest;
  return code;
}

static inline reg_t *dscode_call(const reg_t *func, const reg_t offset) {
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
  reg_t *const code = code_alloc(op - 5, 3);
  code[1] = src;
  code[3] = dest;
  return code;
}

#define dscode_op_imm(a)\
static inline reg_t *dscode_##a##_imm(const reg_t src, const reg_t imm, const reg_t dest) {\
  reg_t *const code = code_alloc(dsop_##a##_imm, 3);\
  code[1] = src;\
  code[2] = imm;\
  code[3] = dest;\
  return code;\
}

dscode_op_imm(add);
dscode_op_imm(sub);
dscode_op_imm(mul);
dscode_op_imm(div);
dscode_op_imm(mod);

static inline reg_t *dscode_lt_jump(const reg_t src, const reg_t imm, const reg_t dest) {
  reg_t *const code = code_alloc(dsop_lt_jump, 3);
  code[1] = src;
  code[2] = imm;
  code[3] = dest;
  return code;
}

#define dscode_op(a) \
static inline reg_t *dscode_##a(const reg_t src, const reg_t imm, const reg_t dest) {\
  reg_t *const code = code_alloc(dsop_##a, 3);\
  code[1] = src;\
  code[2] = imm;\
  code[3] = dest;\
  return code;\
}

dscode_op(add);
dscode_op(sub);
dscode_op(mul);
dscode_op(div);
dscode_op(mod);

dscode_op(eq);
dscode_op(ne);
dscode_op(land);
dscode_op(lor);

dscode_op(ge);
dscode_op(gt);
dscode_op(lt);
dscode_op(le);

dscode_op(band);
dscode_op(bor);
dscode_op(bxor);
dscode_op(blshift);
dscode_op(brshift);

#define _dscode_unary(a) \
static inline reg_t *dscode_##a(const reg_t src, const reg_t dest) {\
  reg_t *const code = code_alloc(dsop_##a, 2);\
  code[1] = src;\
  code[3] = dest;\
  return code;\
}

_dscode_unary(inc);
_dscode_unary(dec);
_dscode_unary(neg);
_dscode_unary(not);
_dscode_unary(cmp);

static inline reg_t *dscode_jump(const reg_t dest) {
  reg_t *const code = code_alloc(dsop_jump, 1);
  code[3] = dest;
  return code;
}

static inline reg_t *dscode_jump_op(const ds_opcode op, const reg_t src, const reg_t imm, const reg_t dest) {
  reg_t *const code = code_alloc(op, 3);
  code[1] = src;
  code[2] = imm;
  code[3] = dest;
  return code;
}

#define dscode_op_jump(a) \
static inline reg_t *dscode_jump_##a(const reg_t src, const reg_t imm, const reg_t dest) {\
  reg_t *const code = code_alloc(dsop_##a, 3);\
  code[1] = src;\
  code[2] = imm;\
  code[3] = dest;\
  return code;\
}

dscode_op_jump(eq);
dscode_op_jump(ne);
dscode_op_jump(gt);
dscode_op_jump(ge);
dscode_op_jump(lt);
dscode_op_jump(le);

reg_t *dscode_start(void);
