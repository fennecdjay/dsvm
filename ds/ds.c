#include <stdio.h>
#include "ds.h"

#define RVM_SIZE 256

#define DISPATCH() { void *next = *(void**)code; code+= sizeof(void*); goto *next;}

#define vm_read() *(reg_t*)code;  code += sizeof(reg_t);

#define BINARY(op) do               \
{                                   \
  const DsInfo3j info = *(DsInfo3j*)code;\
  code += sizeof(DsInfo3j);\
  reg[info.ptr] = reg[info.lhs] op reg[info.rhs]; \
	goto *info.addr;\
} while(0)

#define BINARY_IMM(op) do               \
{                                   \
  const DsInfo3j info = *(DsInfo3j*)code;\
  code += sizeof(DsInfo3j);\
  reg[info.ptr] = reg[info.lhs] op info.rhs; \
	goto *info.addr;\
} while(0)

#define UNARY(op) do                \
{                                   \
  const DsInfoj info = *(DsInfoj*)code;\
  code += sizeof(DsInfoj);\
  reg[info.rhs] = op reg[info.lhs];          \
  goto *info.addr;\
} while(0)

#define BINARY_IF(op)  do { \
  const reg_t lhs  = *(reg_t*)code;\
  const reg_t rhs  = *(reg_t*)(code + sizeof(reg_t));\
  dscode_t *const new_code  = *(dscode_t**)(code + sizeof(reg_t)*2);\
  if(!(reg[lhs] op reg[rhs]))       \
    code = new_code;            \
  else\
    code += 3*sizeof(void*);\
  DISPATCH();\
} while(0)

#define BINARY_IF_IMM(op)  do { \
  const reg_t lhs  = *(reg_t*)code;\
  const reg_t rhs  = *(reg_t*)(code + sizeof(reg_t));\
  dscode_t *const new_code  = *(dscode_t**)(code + sizeof(reg_t)*2);\
  if(!(reg[lhs] op rhs))       \
    code = new_code;            \
  else\
    code += 3*sizeof(void*);\
  DISPATCH();\
} while(0)
/*
#define BINARYF(op) do               \
{                                   \
  const DsInfo3j info = *(DsInfo3j*)code;\
  code += sizeof(DsInfo3j);\
  *(float*)(reg + info.ptr) = *(float*)(reg + info.lhs) op *(float*)(reg + info.rhs); \
  goto *info.addr;\
} while(0)
*/
__attribute__((nonnull(1)))
void dsvm_run(DsThread *thread, const dscode_t *next) {
  dscode_t *code = thread->code;
  reg_t *reg = thread->reg;
  DsFrame *frames = thread->frames;
  uint32_t nframe = thread->nframe;

  if(!next) {
    DISPATCH();
  _dsop_imm:
{
//  const reg_t lhs = vm_read();
//  const reg_t dest = vm_read();
  const reg_t lhs = *(reg_t*)code;
  const reg_t dest = *(reg_t*)(code + sizeof(reg_t));
const void *addr = *(void**)(code + sizeof(reg_t)*2);
code += sizeof(reg_t) *3;
  reg[dest] = lhs;
 goto *addr;
//  DISPATCH();
}
/*
  _dsop_imm_sub:
{
  const reg_t lhs = vm_read();
  const reg_t rhs = vm_read();
  const reg_t dest = vm_read();
  reg[dest] = reg[lhs] - rhs;
  DISPATCH();
}
*/
  _dsop_add:
  BINARY(+);
  _dsop_sub:
  BINARY(-);
  _dsop_mul:
  BINARY(*);
  _dsop_div:
  BINARY(/);
  _dsop_mod:
  BINARY(%);

  _dsop_eq:
  BINARY(==);
  _dsop_ne:
  BINARY(!=);

  _dsop_lt:
  BINARY(<);
  _dsop_le:
  BINARY(<=);
  _dsop_gt:
  BINARY(>);
  _dsop_ge:
  BINARY(>=);


  _dsop_land:
  BINARY(&&);
  _dsop_lor:
  BINARY(||);

  _dsop_band:
  BINARY(&);
  _dsop_bor:
  BINARY(|);
  _dsop_bxor:
  BINARY(^);
  _dsop_shl:
  BINARY(<<);
  _dsop_shr:
  BINARY(>>);


  _dsop_add_imm:
  BINARY_IMM(+);
  _dsop_sub_imm:
  BINARY_IMM(-);
  _dsop_mul_imm:
  BINARY_IMM(*);
  _dsop_div_imm:
  BINARY_IMM(/);
  _dsop_mod_imm:
  BINARY_IMM(%);

  _dsop_eq_imm:
  BINARY_IMM(==);
  _dsop_ne_imm:
  BINARY_IMM(!=);

  _dsop_lt_imm:
  BINARY_IMM(<);
  _dsop_le_imm:
  BINARY_IMM(<=);
  _dsop_gt_imm:
  BINARY_IMM(>);
  _dsop_ge_imm:
  BINARY_IMM(>=);


  _dsop_land_imm:
  BINARY_IMM(&&);
  _dsop_lor_imm:
  BINARY_IMM(||);

  _dsop_band_imm:
  BINARY_IMM(&);
  _dsop_bor_imm:
  BINARY_IMM(|);
  _dsop_bxor_imm:
  BINARY_IMM(^);
  _dsop_shl_imm:
  BINARY_IMM(<<);
  _dsop_shr_imm:
  BINARY_IMM(>>);

  _dsop_goto:
{
  code = (dscode_t*)vm_read();
  DISPATCH();
}

  _dsop_if:
{
  reg_t cond = *(reg_t*)code;
  code = *(dscode_t**)(reg[cond] ?
    code + sizeof(reg_t) :
    code + sizeof(reg_t) + sizeof(void*));
  DISPATCH();
}

  _dsop_if_add:
  BINARY_IF(+);
  _dsop_if_sub:
  BINARY_IF(-);
  _dsop_if_mul:
  BINARY_IF(&&); // from a gcc suggestion
  _dsop_if_div:
  BINARY_IF(/);
  _dsop_if_mod:
  BINARY_IF(%);

  _dsop_if_eq:
  BINARY_IF(==);
  _dsop_if_ne:
  BINARY_IF(!=);

  _dsop_if_lt:
  BINARY_IF(<);
  _dsop_if_le:
  BINARY_IF(<=);
  _dsop_if_gt:
  BINARY_IF(>);
  _dsop_if_ge:
  BINARY_IF(>=);


  _dsop_if_land:
  BINARY_IF(&&);
  _dsop_if_lor:
  BINARY_IF(||);

  _dsop_if_band:
  BINARY_IF(&);
  _dsop_if_bor:
  BINARY_IF(|);
  _dsop_if_bxor:
  BINARY_IF(^);
  _dsop_if_shl:
  BINARY_IF(<); // from a gcc suggestion
  _dsop_if_shr:
  BINARY_IF(>>);

  _dsop_if_add_imm:
  BINARY_IF_IMM(+);
  _dsop_if_sub_imm:
  BINARY_IF_IMM(-);
  _dsop_if_mul_imm:
  BINARY_IF_IMM(&&); // from a gcc suggestion
  _dsop_if_div_imm:
  BINARY_IF_IMM(/);
  _dsop_if_mod_imm:
  BINARY_IF_IMM(%);

  _dsop_if_eq_imm:
  BINARY_IF_IMM(==);
  _dsop_if_ne_imm:
  BINARY_IF_IMM(!=);

  _dsop_if_lt_imm:
  BINARY_IF_IMM(<);
  _dsop_if_le_imm:
  BINARY_IF_IMM(<=);
  _dsop_if_gt_imm:
  BINARY_IF_IMM(>);
  _dsop_if_ge_imm:
  BINARY_IF_IMM(>=);


  _dsop_if_land_imm:
  BINARY_IF_IMM(&&);
  _dsop_if_lor_imm:
  BINARY_IF_IMM(||);

  _dsop_if_band_imm:
  BINARY_IF_IMM(&);
  _dsop_if_bor_imm:
  BINARY_IF_IMM(|);
  _dsop_if_bxor_imm:
  BINARY_IF_IMM(^);
  _dsop_if_shl_imm:
  BINARY_IF_IMM(<); // from a gcc suggestion
  _dsop_if_shr_imm:
  BINARY_IF_IMM(>>);

  _dsop_neg:
  UNARY(-);
  _dsop_not:
  UNARY(!);
  _dsop_cmp:
  UNARY(~);
  _dsop_abs:
{
  const DsInfoj info = *(DsInfoj*)code;
  code += sizeof(DsInfoj);
  reg[info.rhs] = labs(reg[info.lhs]); // arch
  goto *info.addr;
}
  _dsop_mov:
  UNARY();
  _dsop_deref:
  UNARY(*(reg_t*));
  _dsop_addr:
  UNARY((reg_t)&);

  _dsop_call:
{
  const DsInfo3 info = *(DsInfo3*)code;
  if(nframe == RVM_SIZE - 1) exit(13);
  frames[nframe++] = (DsFrame){ .reg = reg, .code = code + sizeof(DsInfo3), .out = info.rhs };
  reg += info.lhs;
  code = info.code;
  DISPATCH();                       \
}
  _dsop_return:
{
  const DsFrame frame = frames[--nframe];
  const reg_t _out    = reg[*(reg_t*)code];
  code = frame.code;
  reg = frame.reg;
  reg[frame.out] = _out;
  DISPATCH();
}
/*
  _dsop_immf:
{
  const float f = vm_readf();
  const reg_t dest = vm_read();
  *(float*)(reg + dest) = f;
  DISPATCH();
}
  _dsop_addf:
  BINARYF(+);
  _dsop_subf:
  BINARYF(-);
  _dsop_mulf:
  BINARYF(*);
  _dsop_divf:
  BINARYF(/);
*/
_dsop_end:
  return;
_dsop_jit:
{
  typedef reg_t (*jitfun_t)(reg_t*);
  jitfun_t trampoline = *(jitfun_t*)code;
  if(trampoline) {
    const reg_t out = trampoline(reg);
    const DsFrame frame = frames[--nframe];
    code = frame.code;
    reg = frame.reg;
    reg[frame.out] = out;
    DISPATCH();
  } else {
    void *addr = *(void**)(code + sizeof(jitfun_t*));
    code += sizeof(jitfun_t*) + sizeof(void*);
    goto *addr;
  }
}
}

  // prepare code
  #define PREPARE(a, b)       \
    __dsop_##a:                     \
    *(void**)code = &&_dsop_##a; \
    code += b * sizeof(reg_t);\
    if(code == next){ return;}            \
    goto *dispatch[*(reg_t*)code];              \

  #define PREPARE2(a, b)       \
    __dsop_##a:                     \
    *(void**)code = &&_dsop_##a; \
    code += sizeof(void*) + sizeof(b);\
    if(code == next){ return;}            \
    goto *dispatch[*(reg_t*)code];              \

  static const void *dispatch[dsop_max] = {
     &&__dsop_imm,

     &&__dsop_add, &&__dsop_sub, &&__dsop_mul,
     &&__dsop_div, &&__dsop_mod,

     &&__dsop_eq, &&__dsop_ne,
     &&__dsop_lt, &&__dsop_le,
     &&__dsop_gt, &&__dsop_ge,

     &&__dsop_land, &&__dsop_lor,
     &&__dsop_band, &&__dsop_bor, &&__dsop_bxor,
     &&__dsop_shl, &&__dsop_shr,

     &&__dsop_add_imm, &&__dsop_sub_imm, &&__dsop_mul_imm,
     &&__dsop_div_imm, &&__dsop_mod_imm,

     &&__dsop_eq_imm, &&__dsop_ne_imm,
     &&__dsop_lt_imm, &&__dsop_le_imm,
     &&__dsop_gt_imm, &&__dsop_ge_imm,

     &&__dsop_land_imm, &&__dsop_lor_imm,
     &&__dsop_band_imm, &&__dsop_bor_imm, &&__dsop_bxor_imm,
     &&__dsop_shl_imm, &&__dsop_shr_imm,

     &&__dsop_goto, &&__dsop_if,

     &&__dsop_if_add, &&__dsop_if_sub, &&__dsop_if_mul,
     &&__dsop_if_div, &&__dsop_if_mod,

     &&__dsop_if_eq, &&__dsop_if_ne,
     &&__dsop_if_lt, &&__dsop_if_le,
     &&__dsop_if_gt, &&__dsop_if_ge,

     &&__dsop_if_land, &&__dsop_if_lor,
     &&__dsop_if_band, &&__dsop_if_bor, &&__dsop_if_bxor,
     &&__dsop_if_shl, &&__dsop_if_shr,

     &&__dsop_if_add_imm, &&__dsop_if_sub_imm, &&__dsop_if_mul_imm,
     &&__dsop_if_div_imm, &&__dsop_if_mod_imm,

     &&__dsop_if_eq_imm, &&__dsop_if_ne_imm,
     &&__dsop_if_lt_imm, &&__dsop_if_le_imm,
     &&__dsop_if_gt_imm, &&__dsop_if_ge_imm,

     &&__dsop_if_land_imm, &&__dsop_if_lor_imm,
     &&__dsop_if_band_imm, &&__dsop_if_bor_imm, &&__dsop_if_bxor_imm,
     &&__dsop_if_shl_imm, &&__dsop_if_shr_imm,

&&__dsop_neg, &&__dsop_not, &&__dsop_cmp, &&__dsop_abs,
&&__dsop_mov, &&__dsop_deref, &&__dsop_addr,

     &&__dsop_call, &&__dsop_return,
/*
     &&__dsop_immf,

     &&__dsop_addf, &&__dsop_subf,
     &&__dsop_mulf, &&__dsop_divf,
*/
    &&__dsop_end,
&&__dsop_jit
  };

  goto *dispatch[*(reg_t*)code];

  PREPARE(imm, 3);

  PREPARE2(add, DsInfo3);
  PREPARE2(sub, DsInfo3); PREPARE2(mul, DsInfo3);
  PREPARE2(div, DsInfo3); PREPARE2(mod, DsInfo3);

  PREPARE2(eq, DsInfo3); PREPARE2(ne, DsInfo3);
  PREPARE2(lt, DsInfo3); PREPARE2(le, DsInfo3);
  PREPARE2(gt, DsInfo3); PREPARE2(ge, DsInfo3);

  PREPARE2(land, DsInfo3); PREPARE2(lor, DsInfo3);
  PREPARE2(band, DsInfo3); PREPARE2(bor, DsInfo3); PREPARE2(bxor, DsInfo3);
  PREPARE2(shl, DsInfo3); PREPARE2(shr, DsInfo3);

  PREPARE2(add_imm, DsInfo3);
  PREPARE2(sub_imm, DsInfo3); PREPARE2(mul_imm, DsInfo3);
  PREPARE2(div_imm, DsInfo3); PREPARE2(mod_imm, DsInfo3);

  PREPARE2(eq_imm, DsInfo3); PREPARE2(ne_imm, DsInfo3);
  PREPARE2(lt_imm, DsInfo3); PREPARE2(le_imm, DsInfo3);
  PREPARE2(gt_imm, DsInfo3); PREPARE2(ge_imm, DsInfo3);

  PREPARE2(land_imm, DsInfo3); PREPARE2(lor_imm, DsInfo3);
  PREPARE2(band_imm, DsInfo3); PREPARE2(bor_imm, DsInfo3); PREPARE2(bxor_imm, DsInfo3);
  PREPARE2(shl_imm, DsInfo3); PREPARE2(shr_imm, DsInfo3);

  PREPARE(goto, 2);
  PREPARE(if, 4);

  PREPARE2(if_add, DsInfo3);
  PREPARE2(if_sub, DsInfo3); PREPARE2(if_mul, DsInfo3);
  PREPARE2(if_div, DsInfo3); PREPARE2(if_mod, DsInfo3);

  PREPARE2(if_eq, DsInfo3); PREPARE2(if_ne, DsInfo3);
  PREPARE2(if_lt, DsInfo3); PREPARE2(if_le, DsInfo3);
  PREPARE2(if_gt, DsInfo3); PREPARE2(if_ge, DsInfo3);

  PREPARE2(if_land, DsInfo3); PREPARE2(if_lor, DsInfo3);
  PREPARE2(if_band, DsInfo3); PREPARE2(if_bor, DsInfo3); PREPARE2(if_bxor, DsInfo3);
  PREPARE2(if_shl, DsInfo3); PREPARE2(if_shr, DsInfo3);


  PREPARE2(if_add_imm, DsInfo3);
  PREPARE2(if_sub_imm, DsInfo3); PREPARE2(if_mul_imm, DsInfo3);
  PREPARE2(if_div_imm, DsInfo3); PREPARE2(if_mod_imm, DsInfo3);

  PREPARE2(if_eq_imm, DsInfo3); PREPARE2(if_ne_imm, DsInfo3);
  PREPARE2(if_lt_imm, DsInfo3); PREPARE2(if_le_imm, DsInfo3);
  PREPARE2(if_gt_imm, DsInfo3); PREPARE2(if_ge_imm, DsInfo3);

  PREPARE2(if_land_imm, DsInfo3); PREPARE2(if_lor_imm, DsInfo3);
  PREPARE2(if_band_imm, DsInfo3); PREPARE2(if_bor_imm, DsInfo3); PREPARE2(if_bxor_imm, DsInfo3);
  PREPARE2(if_shl_imm, DsInfo3); PREPARE2(if_shr_imm, DsInfo3);


  PREPARE2(neg, DsInfo);
  PREPARE2(not, DsInfo);
  PREPARE2(cmp, DsInfo);
  PREPARE2(abs, DsInfo);
  PREPARE2(mov, DsInfo);
  PREPARE2(deref, DsInfo);
  PREPARE2(addr, DsInfo);

  PREPARE2(call, DsInfo3);
  PREPARE(return, 2);

//  PREPARE(immf, 3);
//  PREPARE(addf, 4); PREPARE(subf, 4);
//  PREPARE(mulf, 4); PREPARE(divf, 4);

  PREPARE(end, 1);
  PREPARE(jit, 2);
}

#define BUMP_SIZE 4096

static dscode_t _code[BUMP_SIZE];
static uint32_t _code_count;

dscode_t *dscode_start(void) {
  return _code + _code_count;
}

dscode_t *code_alloc(const ds_opcode op, const uint32_t n) {
  dscode_t *const code = _code + _code_count;
  *(reg_t*)code = op;
  _code_count += (n + 1) * sizeof(reg_t);
  return code + sizeof(void*);
}

dscode_t *code_alloc2(const ds_opcode op, const uint32_t n) {
  dscode_t *const code = _code + _code_count;
  *(reg_t*)code = op;
  _code_count += (n + sizeof(void*));
  return code + sizeof(void*);
}
