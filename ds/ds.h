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

  dsop_eq, dsop_ne,
  dsop_lt, dsop_le,
  dsop_gt, dsop_ge,

  dsop_land, dsop_lor,

  dsop_band,dsop_bor, dsop_bxor,
  dsop_blshift, dsop_brshift,

  dsop_inc, dsop_dec, dsop_mov,
  dsop_neg, dsop_not, dsop_cmp,
// miss abs
  dsop_jump,
  dsop_eq_jump, dsop_ne_jump,
  dsop_lt_jump, dsop_le_jump,
  dsop_gt_jump, dsop_ge_jump,

  dsop_call, dsop_return,
  dsop_call2,

  dsop_immf,

  dsop_add_immf, dsop_sub_immf,
  dsop_mul_immf, dsop_div_immf,

  dsop_addf, dsop_subf,
  dsop_mulf, dsop_divf,

  dsop_end,
  dsop_max,
} ds_opcode;

typedef struct {
  reg_t *code;
  reg_t  *reg;
//  reg_t  out;
  uint32_t  out;
} Frame;

//ANN 

__attribute__((nonnull(1)))
//void dsvm_run(reg_t *, const reg_t *);
void dsvm_run(char *, const char *);

reg_t *code_alloc(const ds_opcode, const uint32_t);
reg_t *code_alloc2(const ds_opcode, const uint32_t);

static inline reg_t *dscode_imm(const reg_t imm, const uint32_t dest) {
  char *const code = code_alloc2(dsop_imm, sizeof(reg_t)+ sizeof(uint32_t));
  *(reg_t*)(code + sizeof(void*)) = imm;
  *(uint32_t*)(code + sizeof(void*) + sizeof(reg_t)) = dest;
  return (reg_t*)code;
}

ANN static inline reg_t *dscode_call(const reg_t *func, const uint32_t offset, const uint32_t dest) {
  char *const code = code_alloc2(dsop_call, sizeof(reg_t) + 2 * sizeof(uint32_t));
  *(reg_t**)(code + sizeof(void*)) = func;
  *(uint32_t*)(code + sizeof(void*) + sizeof(reg_t*)) = offset;
  *(uint32_t*)(code + sizeof(void*) + sizeof(reg_t*) + sizeof(uint32_t)) = dest;
  return code;
}

ANN static inline reg_t *dscode_call2(const reg_t *func, const reg_t arg1, const reg_t arg2, const reg_t out ) {
  reg_t *const code = code_alloc(dsop_call2, 4);
  code[1] = (reg_t)func;
  code[2] = arg1;
  code[3] = arg2;
  code[4] = out;
  return code;
}

static inline reg_t *dscode_return(const reg_t dest) {
  reg_t *const code = code_alloc(dsop_return, 1);
  code[1] = dest;
  return code;
}

/*
//static inline reg_t *dscode_return(const reg_t dest) {
static inline reg_t *dscode_return() {
  reg_t *const code = code_alloc(dsop_return, 1);
//  code[1] = (reg_t)dest;
  return code;
}
*/
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

static inline reg_t *dscode_immf(const float imm, const reg_t dest) {
  reg_t *const code = code_alloc(dsop_immf, 2);
  *(float*)(code + 1) = imm;
  code[2] = dest;
  return code;
}


reg_t *dscode_start(void);
