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

  dsop_add, dsop_sub, dsop_mul,
  dsop_div, dsop_mod,

  dsop_eq, dsop_ne,
  dsop_lt, dsop_le,
  dsop_gt, dsop_ge,

  dsop_land, dsop_lor,

  dsop_band, dsop_bor, dsop_bxor,
  dsop_shl, dsop_shr,

  /* unary ops */
//  dsop_neg, dsop_lnot, dsop_bnot, dsop_abs

  dsop_if, dsop_goto,
  dsop_call, dsop_return,

//  dsop_immf,
//  dsop_addf, dsop_subf,
//  dsop_mulf, dsop_divf,
  dsop_end,
  dsop_max,
} ds_opcode;

typedef struct {
  dscode_t *code;
  reg_t  *reg;
//  reg_t  out;
  dsidx_t  out;
} DsFrame;

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

ANN static inline dscode_t *dscode_goto(const dscode_t *new_code) {
  dscode_t *const code = code_alloc(dsop_goto, 1);
  *(const dscode_t**)code = new_code;
  return code;
}

/*
// humm
static inline dscode_t *dscode_immf(const float imm, const reg_t dest) {
  dscode_t *const code = code_alloc(dsop_immf, 2);
  *(float*)(code + sizeof(void*)) = imm;
  *(reg_t*)(code + sizeof(void*) + sizeof(float)) = dest;
  code[2] = dest;
  return code;
}
*/

static inline dscode_t *dscode_end(void) {
  return code_alloc(dsop_end, 0);
}

dscode_t *dscode_start(void);
