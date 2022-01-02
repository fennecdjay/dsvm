#include <stdio.h>
#include "ds.h"

typedef reg_t (*dscall2)(reg_t, reg_t);

#define _SIZE 4096

#define RVM_SIZE _SIZE

//#define DISPATCH() goto *(void*)*code++
#define DISPATCH() { void *next = *(void**)code; code+= sizeof(void*); goto *next;}

//#define vm_read() (*((reg_t*)code)++)
#define vm_read() *(reg_t*)code;  code += sizeof(reg_t);
#define vm_read32() *(uint32_t*)code;  code += sizeof(uint32_t);
//#define vm_readf() (*(float*)code++)
#define vm_readf() *(float*)code;  code += sizeof(reg_t);

#define BINARY_IMM(op) do           \
{                                   \
  const reg_t lhs  = vm_read();     \
  const reg_t rhs  = vm_read();     \
  const reg_t dest = vm_read();     \
  reg[dest] = reg[lhs] op rhs;      \
  DISPATCH();                       \
} while(0)

#define BINARY(op) do               \
{                                   \
  const reg_t lhs = vm_read();      \
  const reg_t rhs = vm_read();      \
  const reg_t dest = vm_read();     \
  reg[dest] = reg[lhs] op reg[rhs]; \
  DISPATCH();                       \
} while(0)

#define UNARY(op) do                \
{                                   \
  const reg_t lhs = vm_read();      \
  const reg_t dest = vm_read();     \
  reg[dest] = op reg[lhs];          \
  DISPATCH();                       \
} while(0)

#define JUMP(op) do              \
{                                   \
  const reg_t lhs = vm_read();      \
  const reg_t rhs = vm_read();      \
  const reg_t dest = vm_read();     \
  if((reg[lhs] op reg[rhs]))       \
    code = (reg_t*)dest;            \
  DISPATCH();                       \
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
  const reg_t lhs = vm_read();      \
  const reg_t rhs = vm_read();      \
  const reg_t dest = vm_read();     \
  *(float*)(reg + dest) = *(float*)(reg + lhs) op *(float*)(reg + rhs); \
  DISPATCH();                       \
} while(0)

__attribute__((nonnull(1)))
//void dsvm_run(reg_t *code, const reg_t *next) {
void dsvm_run(char *code, const char *next) {
  static reg_t _reg[RVM_SIZE];
  Frame frames[RVM_SIZE];
  reg_t *reg = _reg;
reg_t ret;
  uint32_t nframe = 0;

  if(!next) {
    DISPATCH();
  _dsop_imm:
{
  const reg_t lhs = vm_read();
//  const reg_t dest = vm_read();
  const reg_t dest = vm_read32();
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
  reg_t *const new_code = (reg_t*)vm_read();
  const reg_t offset = vm_read32();
  const reg_t out = vm_read32();
  frames[nframe++] = (Frame){ .reg = reg, .code = code, .out = out };
  reg += offset;
  code = new_code;
  DISPATCH();
}
  _dsop_return:
{
  const reg_t _out = vm_read();
  const reg_t out = reg[_out];
  const Frame frame = frames[--nframe];
  code = frame.code;
  reg = frame.reg;

  reg[frame.out] = out;
//  *(reg_t*)frame.out = out;
//  ret = out;

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
    code += b;\
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

//  PREPARE(imm, 3);
//  PREPARE2(imm, 3  *sizeof(reg_t));
  PREPARE2(imm, 2 * sizeof(reg_t) + sizeof(uint32_t));

  PREPARE(add_imm, 4); PREPARE(sub_imm, 4);
  PREPARE(mul_imm, 4); PREPARE(div_imm, 4); PREPARE(mod_imm, 4);

  PREPARE(add, 4); PREPARE(sub, 4);
  PREPARE(mul, 4); PREPARE(div, 4); PREPARE(mod, 4);

  PREPARE(eq, 4); PREPARE(ne, 4);
  PREPARE(lt, 4); PREPARE(le, 4);
  PREPARE(gt, 4); PREPARE(ge, 4);

  PREPARE(land, 4); PREPARE(lor, 4);
  PREPARE(band, 4); PREPARE(bor, 4); PREPARE(bxor, 4);
  PREPARE(blshift, 4); PREPARE(brshift, 4);

  PREPARE(inc, 3); PREPARE(dec, 3); PREPARE(mov, 3);
  PREPARE(neg, 3); PREPARE(not, 3); PREPARE(cmp, 3);

  PREPARE(jump, 2);
  PREPARE(eq_jump, 4);
  PREPARE(ne_jump, 4);
  PREPARE(lt_jump, 4);
  PREPARE(le_jump, 4);
  PREPARE(gt_jump, 4);
  PREPARE(ge_jump, 4);

//  PREPARE(call, 4);
  PREPARE2(call, 2 * sizeof(reg_t*) + 2 * sizeof(uint32_t));
  PREPARE(return, 2);

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
  return code;
}

reg_t *code_alloc2(const ds_opcode op, const uint32_t n) {
  char *const c = _code;
  reg_t *const code = c + _code_count;
  *(reg_t*)code = op;
  _code_count += (n + sizeof(void*));
  return code;
}
