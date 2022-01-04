#include <stdio.h>
#include "ds.h"

typedef reg_t (*dscall2)(reg_t, reg_t);

#define _SIZE 4096

#define RVM_SIZE _SIZE

//#define DISPATCH() goto **code++
#define DISPATCH() { void *next = *(void**)code; code+= sizeof(void*); goto *next;}

#define vm_read() *(reg_t*)code;  code += sizeof(reg_t);
#define vm_readf() *(float*)code;  code += sizeof(reg_t);

#define BINARY_IMM(op) do           \
{                                   \
  const DsInfo3j info = *(DsInfo3j*)code;\
  code += sizeof(DsInfo3j);\
  reg[info.rhs] = reg[info.lhs] op info.ptr;     \
	goto *info.addr;\
} while(0)

#define BINARY(op) do               \
{                                   \
  const DsInfo3j info = *(DsInfo3j*)code;\
  code += sizeof(DsInfo3j);\
  reg[info.ptr] = reg[info.lhs] op reg[info.rhs]; \
	goto *info.addr;\
} while(0)

#define UNARY(op) do                \
{                                   \
  const DsInfoj info = *(DsInfoj*)code;\
  code += sizeof(DsInfoj);\
  reg[info.rhs] = op reg[info.lhs];          \
  goto *info.addr;\
} while(0)

#define JUMP(op) do                 \
{                                   \
  const DsInfo3j info = *(DsInfo3j*)code;\
  if(reg[info.lhs] op reg[info.rhs])          {\
    code = (reg_t*)info.ptr;            \
    DISPATCH();\
  };                       \
  code += sizeof(DsInfo3j);\
	goto *info.addr;\
} while(0)

#define BINARY_IMMF(op) do          \
{                                   \
  const reg_t lhs  = vm_read();     \
  const float rhs  = vm_readf();     \
  const reg_t dest = vm_read();     \
  *(float*)(reg + dest) = *(float*)(reg+lhs) op rhs;      \
  DISPATCH();                       \
} while(0)

#define BINARYF(op) do               \
{                                   \
  const DsInfo3j info = *(DsInfo3j*)code;\
  code += sizeof(DsInfo3j);\
  *(float*)(reg + info.ptr) = *(float*)(reg + info.lhs) op *(float*)(reg + info.rhs); \
  goto *info.addr;\
} while(0)

__attribute__((nonnull(1)))
//void dsvm_run(reg_t *code, const reg_t *next) {
void dsvm_run(char *code, const char *next) {
  static reg_t _reg[RVM_SIZE];
  Frame frames[RVM_SIZE];
  reg_t *reg = _reg;
//reg_t ret;
  uint32_t nframe = 0;

  if(!next) {
    DISPATCH();
  _dsop_imm:
{
  const reg_t lhs = vm_read();
  const reg_t dest = vm_read();
  reg[dest] = lhs;
  DISPATCH();
}

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
  _dsop_blshift:
  BINARY(<<);
  _dsop_brshift:
  BINARY(>>);

  _dsop_inc:
  UNARY(++);
  _dsop_dec:
  UNARY(--);
  _dsop_mov:
{
  const reg_t lhs = vm_read();
  const reg_t dest = vm_read();
  reg[dest] = reg[lhs];
  DISPATCH();
}
  _dsop_neg:
  UNARY(-);
  _dsop_not:
  UNARY(!);
  _dsop_cmp:
  UNARY(~);

  _dsop_jump:
{
  const reg_t dest = vm_read();
  code = (reg_t*)dest;
  DISPATCH();
}
  _dsop_eq_jump:
  JUMP(==);
  _dsop_ne_jump:
  JUMP(!=);
  _dsop_lt_jump:
  JUMP(<);
  _dsop_le_jump:
  JUMP(<=);
  _dsop_gt_jump:
  JUMP(>);
  _dsop_ge_jump:
  JUMP(>=);



  _dsop_call:
{
  const DsInfo3 info = *(DsInfo3*)code;
  frames[nframe++] = (Frame){ .reg = reg, .code = code + sizeof(DsInfo3), .out = info.rhs };
  reg += info.lhs;
  code = info.ptr;
  DISPATCH();                       \
}
  _dsop_return:
{
  const Frame frame = frames[--nframe];
  const reg_t _out = vm_read();
  reg[frame.out] = reg[_out];
  code = frame.code;
  reg = frame.reg;
  DISPATCH();
}
  _dsop_call2:
{
  const dscall2 ccode = (dscall2)vm_read();
  const reg_t arg0 = vm_read();
  const reg_t arg1 = vm_read();
  const reg_t dest = vm_read();
  reg[dest] = (*ccode)(arg0, arg1);
  DISPATCH();
}

  _dsop_immf:
{
  const float f = vm_readf();
  const reg_t dest = vm_read();
  *(float*)(reg + dest) = f;
  DISPATCH();
}
  _dsop_add_immf:
  BINARY_IMMF(+);
  _dsop_sub_immf:
  BINARY_IMMF(-);
  _dsop_mul_immf:
  BINARY_IMMF(*);
  _dsop_div_immf:
  BINARY_IMMF(/);

  _dsop_addf:
  BINARYF(+);
  _dsop_subf:
  BINARYF(-);
  _dsop_mulf:
  BINARYF(*);
  _dsop_divf:
  BINARYF(/);

  _dsop_end:
  printf("result %lu\n", reg[0]);
  return;
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

     &&__dsop_add_imm, &&__dsop_sub_imm, &&__dsop_mul_imm,
     &&__dsop_div_imm, &&__dsop_mod_imm,

     &&__dsop_add, &&__dsop_sub, &&__dsop_mul,
     &&__dsop_div, &&__dsop_mod,

     &&__dsop_eq, &&__dsop_ne,
     &&__dsop_lt, &&__dsop_le, &&__dsop_gt, &&__dsop_ge,

     &&__dsop_land, &&__dsop_lor,
     &&__dsop_band, &&__dsop_bor, &&__dsop_bxor,
     &&__dsop_blshift, &&__dsop_brshift,

     &&__dsop_inc, &&__dsop_dec, &&__dsop_mov,
     &&__dsop_neg, &&__dsop_not, &&__dsop_cmp,

     &&__dsop_jump,
     &&__dsop_eq_jump, &&__dsop_ne_jump,
     &&__dsop_lt_jump, &&__dsop_le_jump,
     &&__dsop_gt_jump, &&__dsop_ge_jump,

     &&__dsop_call, &&__dsop_return,
     &&__dsop_call2,
     &&__dsop_immf,

     &&__dsop_add_immf, &&__dsop_sub_immf,
     &&__dsop_mul_immf, &&__dsop_div_immf,

     &&__dsop_addf, &&__dsop_subf,
     &&__dsop_mulf, &&__dsop_divf,

     &&__dsop_end,
  };
  goto *dispatch[*code];

  PREPARE(imm, 3);

  PREPARE2(add_imm, DsInfo3);
  PREPARE2(sub_imm, DsInfo3);
  PREPARE2(mul_imm, DsInfo3);
  PREPARE2(div_imm, DsInfo3);
  PREPARE2(mod_imm, DsInfo3);

  PREPARE2(add, DsInfo3);
  PREPARE2(sub, DsInfo3);
  PREPARE2(mul, DsInfo3);
  PREPARE2(div, DsInfo3);
  PREPARE2(mod, DsInfo3);

  PREPARE2(eq, DsInfo3);
  PREPARE2(ne, DsInfo3);
  PREPARE2(lt, DsInfo3);
  PREPARE2(le, DsInfo3);
  PREPARE2(gt, DsInfo3);
  PREPARE2(ge, DsInfo3);

  PREPARE2(land, DsInfo3); PREPARE2(lor, DsInfo3);
  PREPARE2(band, DsInfo3); PREPARE2(bor, DsInfo3); PREPARE2(bxor, DsInfo3);
  PREPARE2(blshift, DsInfo3); PREPARE2(brshift, DsInfo3);

  PREPARE(inc, 3); PREPARE(dec, 3); PREPARE(mov, 3);
  PREPARE(neg, 3); PREPARE(not, 3); PREPARE(cmp, 3);

  PREPARE(jump, 2);


  PREPARE2(eq_jump, DsInfo3);
  PREPARE2(ne_jump, DsInfo3);
  PREPARE2(lt_jump, DsInfo3);
  PREPARE2(le_jump, DsInfo3);
  PREPARE2(gt_jump, DsInfo3);
  PREPARE2(ge_jump, DsInfo3);

  PREPARE2(call, DsInfo3);
  PREPARE(return, 2);
//  PREPARE(return, 1);

  PREPARE(call2, 5);

  PREPARE(immf, 3);
  PREPARE(add_immf, 4);
  PREPARE(sub_immf, 4);
  PREPARE(mul_immf, 4);
  PREPARE(div_immf, 4);
  PREPARE(addf, 4);
  PREPARE(subf, 4);
  PREPARE(mulf, 4);
  PREPARE(divf, 4);

  PREPARE(end, 1);
}

#define BUMP_SIZE _SIZE

static reg_t _code[BUMP_SIZE];
static size_t _code_count = 0;

reg_t *dscode_start(void) {
  char *const c = _code;
  return c + _code_count;
}

reg_t *code_alloc(const ds_opcode op, const uint32_t n) {
  char *const c = _code;
  reg_t *const code = c + _code_count;
  *(reg_t*)code = op;
  _code_count += (n + 1) * sizeof(reg_t);
  return (reg_t)code;
}

char *code_alloc2(const ds_opcode op, const uint32_t n) {
  char *const c = _code;
  reg_t *const code = c + _code_count;
  *(reg_t*)code = op;
  _code_count += (n + sizeof(void*));
  return (reg_t)code;
}
