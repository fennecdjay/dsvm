#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define ANN       __attribute__((nonnull))

typedef uintptr_t reg_t;
typedef uint8_t dscode_t;
typedef uint32_t dsidx_t;

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
  dscode_t *code;
  reg_t  *reg;
//  reg_t  out;
  dsidx_t  out;
} Frame;

//ANN 
typedef struct DsInfo {
  dsidx_t lhs;
  dsidx_t rhs;
} DsInfo;
typedef struct DsInfoj {
  dsidx_t lhs;
  dsidx_t rhs;
  void *addr;
} DsInfoj;

typedef struct DsInfo3 {
  dsidx_t lhs;
  dsidx_t rhs;
  union {
    reg_t ptr;
    dscode_t *code;
  };
} DsInfo3;

typedef struct DsInfo3j {
  dsidx_t lhs;
  dsidx_t rhs;
  union {
    reg_t ptr;
    dscode_t *code;
  };
  void *addr;
} DsInfo3j;

__attribute__((nonnull(1)))
void dsvm_run(dscode_t *, const dscode_t *);

dscode_t *code_alloc(const ds_opcode, const uint32_t);
dscode_t *code_alloc2(const ds_opcode, const uint32_t);

static inline dscode_t *dscode_imm(const reg_t imm, const dsidx_t dest) {
  dscode_t *const code = code_alloc(dsop_imm, 2);
  *(reg_t*)(code + sizeof(void*)) = imm;
  *(dsidx_t*)(code + sizeof(void*) + sizeof(reg_t)) = dest;
  return code;
}

ANN static inline dscode_t *dscode_call(const dscode_t *func, const dsidx_t offset, const dsidx_t dest) {
  dscode_t *const code = code_alloc2(dsop_call, sizeof(DsInfo3));
  *(DsInfo3*)(code + sizeof(void*)) = (DsInfo3){ .ptr = (reg_t)func, .lhs = offset, .rhs = dest };
  return code;
}

ANN static inline reg_t *dscode_call2(const reg_t *func, const reg_t arg1, const reg_t arg2, const reg_t out ) {
  reg_t *const code = (reg_t*)code_alloc(dsop_call2, 4);
  code[1] = (reg_t)func;
  code[2] = arg1;
  code[3] = arg2;
  code[4] = out;
  return code;
}

static inline dscode_t *dscode_return(const reg_t dest) {
  dscode_t *const code = code_alloc(dsop_return, 1);
  *(reg_t*)(code + sizeof(void*)) = dest;
  return code;
}

static inline dscode_t *dscode_end(void) {
  return code_alloc(dsop_end, 0);
}

static inline dscode_t *dscode_binary(const ds_opcode op, const dsidx_t src, const dsidx_t imm, const reg_t dest) {\
  dscode_t *const code = code_alloc2(op, sizeof(DsInfo3));
  *(DsInfo3*)(code + sizeof(void*)) = (DsInfo3){ .ptr=dest, .lhs=src, .rhs=imm };
  return code;
}

static inline dscode_t *dscode_ibinary(const ds_opcode op, const dsidx_t src, const reg_t imm, const dsidx_t dest) {\
  dscode_t *const code = code_alloc2(op - dsop_add + dsop_add_imm, sizeof(DsInfo3));
  *(DsInfo3*)(code + sizeof(void*)) = (DsInfo3){ .ptr = imm, .lhs=src, .rhs=dest };
  return code;
}

static inline dscode_t *dscode_unary(const ds_opcode op, const reg_t src, const reg_t dest) {
  dscode_t *const code = code_alloc2(op, sizeof(DsInfo));
  *(DsInfo*)(code + sizeof(void*)) = (DsInfo){ .lhs=src, .rhs=dest };
  return code;
}

ANN static inline dscode_t *dscode_jump_op(const ds_opcode op, const dsidx_t src, const dsidx_t imm, const dscode_t *new_code) {
  if(op == dsop_jump) {
    dscode_t *const code = code_alloc(dsop_jump, 1);
    *(const dscode_t**)(code + sizeof(void*)) = new_code;
    return code;
  }
  dscode_t *const code = code_alloc2(op, sizeof(DsInfo3));
  *(DsInfo3*)(code + sizeof(void*)) = (DsInfo3){ .ptr = (reg_t)new_code, .lhs=src, .rhs=imm };
  return code;
}

static inline dscode_t *dscode_immf(const float imm, const reg_t dest) {
  dscode_t *const code = code_alloc(dsop_immf, 2);
  *(float*)(code + sizeof(void*)) = imm;
  *(reg_t*)(code + sizeof(void*) + sizeof(float)) = dest;
  code[2] = dest;
  return code;
}

dscode_t *dscode_start(void);
