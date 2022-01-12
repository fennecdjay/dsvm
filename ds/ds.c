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

#define UNARY(op) do                \
{                                   \
  const DsInfoj info = *(DsInfoj*)code;\
  code += sizeof(DsInfoj);\
  reg[info.rhs] = op reg[info.lhs];          \
  goto *info.addr;\
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
  Frame *frames = thread->frames;
  uint32_t nframe = thread->nframe;

  if(!next) {
    DISPATCH();
  _dsop_imm:
{
  const reg_t lhs = vm_read();
  const reg_t dest = vm_read();
  reg[dest] = lhs;
  DISPATCH();
}

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

//  _dsop_eq:
//  BINARY(==);
//  _dsop_ne:
//  BINARY(!=);

  _dsop_lt:
  BINARY(<);
  _dsop_le:
  BINARY(<=);
  _dsop_gt:
  BINARY(>);
  _dsop_ge:
  BINARY(>=);


//  _dsop_land:
//  BINARY(&&);
//  _dsop_lor:
//  BINARY(||);

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
/*
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
*/
  _dsop_if:
{
  reg_t cond = *(reg_t*)code;
  code = *(dscode_t**)(reg[cond] ?
    code + sizeof(reg_t) :
    code + sizeof(reg_t) + sizeof(void*));
  DISPATCH();
}

  _dsop_goto:
{
  code = (dscode_t*)vm_read();
  DISPATCH();
}
  _dsop_call:
{
  const DsInfo3 info = *(DsInfo3*)code;
  if(nframe == RVM_SIZE - 1) exit(3);
  frames[nframe++] = (Frame){ .reg = reg, .code = code + sizeof(DsInfo3), .out = info.rhs };
  reg += info.lhs;
  code = info.code;
  DISPATCH();                       \
}
  _dsop_return:
{
  const Frame frame = frames[--nframe];
  const reg_t _out = reg[*(reg_t*)code];
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

//     &&__dsop_eq, &&__dsop_ne,
     &&__dsop_lt, &&__dsop_le,
     &&__dsop_gt, &&__dsop_ge,

//     &&__dsop_land, &&__dsop_lor,
     &&__dsop_band, &&__dsop_bor, &&__dsop_bxor,
     &&__dsop_shl, &&__dsop_shr,

//     &&__dsop_inc, &&__dsop_dec, &&__dsop_mov,
//     &&__dsop_neg, &&__dsop_not, &&__dsop_cmp,

     &&__dsop_if, &&__dsop_goto,
     &&__dsop_call, &&__dsop_return,
/*
     &&__dsop_immf,

     &&__dsop_addf, &&__dsop_subf,
     &&__dsop_mulf, &&__dsop_divf,
*/
    &&__dsop_end
  };

  goto *dispatch[*(reg_t*)code];

  PREPARE(imm, 3);

  PREPARE2(add, DsInfo3);
  PREPARE2(sub, DsInfo3); PREPARE2(mul, DsInfo3);
  PREPARE2(div, DsInfo3); PREPARE2(mod, DsInfo3);

//  PREPARE2(eq, DsInfo3); PREPARE2(ne, DsInfo3);
  PREPARE2(lt, DsInfo3); PREPARE2(le, DsInfo3);
  PREPARE2(gt, DsInfo3); PREPARE2(ge, DsInfo3);

//  PREPARE2(land, DsInfo3); PREPARE2(lor, DsInfo3);
  PREPARE2(band, DsInfo3); PREPARE2(bor, DsInfo3); PREPARE2(bxor, DsInfo3);
  PREPARE2(shl, DsInfo3); PREPARE2(shr, DsInfo3);

//  PREPARE(inc, 3); PREPARE(dec, 3); PREPARE(mov, 3);
//  PREPARE(neg, 3); PREPARE(not, 3); PREPARE(cmp, 3);

  PREPARE(if, 4);
  PREPARE(goto, 2);


  PREPARE2(call, DsInfo3);
  PREPARE(return, 2);

//  PREPARE(immf, 3);
//  PREPARE(addf, 4); PREPARE(subf, 4);
//  PREPARE(mulf, 4); PREPARE(divf, 4);

  PREPARE(end, 1);
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
