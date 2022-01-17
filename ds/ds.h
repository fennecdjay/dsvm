#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define ANN __attribute__((nonnull))

typedef intptr_t reg_t;
typedef uint8_t dscode_t;
typedef uint32_t dsidx_t;

typedef enum ds_opcode {
  dsop_imm,

  dsop_add, dsop_sub, dsop_mul,
  dsop_div, dsop_mod,

  dsop_eq, dsop_ne,
  dsop_lt, dsop_le,
  dsop_gt, dsop_ge,

  dsop_land, dsop_lor,

  dsop_band, dsop_bor, dsop_bxor,
  dsop_shl, dsop_shr,

  dsop_add_imm, dsop_sub_imm, dsop_mul_imm,
  dsop_div_imm, dsop_mod_imm,

  dsop_eq_imm, dsop_ne_imm,
  dsop_lt_imm, dsop_le_imm,
  dsop_gt_imm, dsop_ge_imm,

  dsop_land_imm, dsop_lor_imm,

  dsop_band_imm, dsop_bor_imm, dsop_bxor_imm,
  dsop_shl_imm, dsop_shr_imm,

  dsop_goto, dsop_if,

  dsop_if_add, dsop_if_sub, dsop_if_mul,
  dsop_if_div, dsop_if_mod,

  dsop_if_eq, dsop_if_ne,
  dsop_if_lt, dsop_if_le,
  dsop_if_gt, dsop_if_ge,

  dsop_if_land, dsop_if_lor,

  dsop_if_band, dsop_if_bor, dsop_if_bxor,
  dsop_if_shl, dsop_if_shr,

  dsop_if_add_imm, dsop_if_sub_imm, dsop_if_mul_imm,
  dsop_if_div_imm, dsop_if_mod_imm,

  dsop_if_eq_imm, dsop_if_ne_imm,
  dsop_if_lt_imm, dsop_if_le_imm,
  dsop_if_gt_imm, dsop_if_ge_imm,

  dsop_if_land_imm, dsop_if_lor_imm,

  dsop_if_band_imm, dsop_if_bor_imm, dsop_if_bxor_imm,
  dsop_if_shl_imm, dsop_if_shr_imm,

  dsop_neg, dsop_not, dsop_cmp, dsop_abs,
  dsop_mov, dsop_deref, dsop_addr,

  dsop_call, dsop_return,

  dsop_end,
  dsop_jit,
  dsop_max,
} ds_opcode;

typedef struct {
  dscode_t *code;
  reg_t  *reg;
  dsidx_t  out;
} DsFrame;

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
  reg_t rhs;
  union {
    reg_t ptr;
    dscode_t *code;
  };
} DsInfo3;

typedef struct DsInfo3j {
  dsidx_t lhs;
  reg_t rhs;
  union {
    reg_t ptr;
    dscode_t *code;
  };
  void *addr;
} DsInfo3j;

typedef struct DsThread {
  dscode_t *code;
  reg_t *reg;
  DsFrame *frames;
  uint32_t nframe;
} DsThread;


__attribute__((nonnull(1)))
void dsvm_run(DsThread *const, const dscode_t *);

dscode_t *code_alloc(const ds_opcode, const uint32_t);
dscode_t *code_alloc2(const ds_opcode, const uint32_t);

static inline dscode_t *dscode_imm(const reg_t imm, const dsidx_t dest) {
  dscode_t *const code = code_alloc(dsop_imm, 2);
  *(reg_t*)code = imm;
  *(dsidx_t*)(code + sizeof(reg_t)) = dest;
  return code;
}

ANN static inline dscode_t *dscode_call(const dscode_t *func, const dsidx_t offset, const dsidx_t dest) {
  dscode_t *const code = code_alloc2(dsop_call, sizeof(DsInfo3));
  *(DsInfo3*)code = (DsInfo3){ .ptr = (reg_t)func, .lhs = offset, .rhs = dest };
  return code;
}

static inline dscode_t *dscode_return(const reg_t dest) {
  dscode_t *const code = code_alloc(dsop_return, 1);
  *(reg_t*)code = dest;
  return code;
}

static inline dscode_t *dscode_binary(const ds_opcode op, const dsidx_t src, const dsidx_t imm, const reg_t dest) {\
  dscode_t *const code = code_alloc2(op, sizeof(DsInfo3));
  *(DsInfo3*)code = (DsInfo3){ .ptr=dest, .lhs=src, .rhs=imm };
  return code;
}

static inline dscode_t *dscode_unary(const ds_opcode op, const reg_t src, const reg_t dest) {
  dscode_t *const code = code_alloc2(op, sizeof(DsInfo));
  *(DsInfo*)code = (DsInfo){ .lhs=src, .rhs=dest };
  return code;
}

ANN static inline dscode_t *dscode_if(const dsidx_t reg) {
  dscode_t *const code = code_alloc(dsop_if, 3);
  *(reg_t*)(code) = reg;
  return code;
}

ANN static inline void dscode_set_if(dscode_t *const code, const dscode_t *_if) {
  *(const dscode_t**)(code + sizeof(void*) + sizeof(reg_t)) = _if;
}
ANN static inline void dscode_set_else(dscode_t *const code, const dscode_t *_else) {
  *(const dscode_t**)(code + sizeof(void*) + sizeof(reg_t) + sizeof(dscode_t*)) = _else;
}

static inline dscode_t *dscode_goto() {
  dscode_t *const code = code_alloc(dsop_goto, 1);
  return code;
}

ANN static inline void dscode_set_goto(dscode_t *const code, const dscode_t *new_code) {
  *(const dscode_t**)(code + sizeof(void*)) = new_code;
}

ANN static inline dscode_t *dscode_if_lt(const reg_t lhs, const reg_t rhs) {
  dscode_t *const code = code_alloc(dsop_if_lt, 3);
  *(reg_t*)(code) = lhs;
  *(reg_t*)(code + sizeof(reg_t)) = rhs;
  return code;
}

ANN static inline dscode_t *dscode_if_op(const ds_opcode op, const reg_t lhs, const reg_t rhs) {
  dscode_t *const code = code_alloc(op, 3);
  *(reg_t*)(code) = lhs;
  *(reg_t*)(code + sizeof(reg_t)) = rhs;
  return code;
}

ANN static inline dscode_t *dscode_if_lt_imm(const reg_t lhs, const reg_t rhs) {
  dscode_t *const code = code_alloc(dsop_if_lt_imm, 3);
  *(reg_t*)(code) = lhs;
  *(reg_t*)(code + sizeof(reg_t)) = rhs;
  return code;
}

ANN static inline void dscode_if_dest(dscode_t *const code, const dscode_t *_else) {
  *(const dscode_t**)(code + sizeof(void*) + sizeof(reg_t)*2) = _else;
}

static inline dscode_t *dscode_end(void) {
  return code_alloc(dsop_end, 0);
}

static inline dscode_t *dscode_jit(void) {
  return code_alloc(dsop_jit, 1);
}

dscode_t *dscode_start(void);
