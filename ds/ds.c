#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "ds.h"

#define BUMP_SIZE 4096

static reg_t _code[BUMP_SIZE];
static size_t _code_count;

#define RVM_SIZE 4096

#define DISPATCH() goto *dispatch[vm_read()];

#define vm_read() (data[pc++])

#define BINARY_IMM(op) do                \
{                                       \
  const reg_t lhs  = vm_read(); \
  const reg_t rhs  = vm_read(); \
  const reg_t dest = vm_read(); \
  reg[dest] = reg[lhs] op rhs;           \
  DISPATCH();                           \
} while(false)

#define BINARY(op) do                    \
{                                       \
  const reg_t lhs = vm_read(); \
  const reg_t rhs = vm_read(); \
  const reg_t dest = vm_read(); \
  reg[dest] = reg[lhs] op reg[rhs];      \
  DISPATCH();                           \
} while(false)

#define UNARY(op) do                    \
{                                       \
  const reg_t lhs = vm_read(); \
  const reg_t dest = vm_read(); \
  reg[dest] = op reg[lhs];               \
  DISPATCH();                           \
} while(false)

#define BRANCH(op) do                   \
{                                       \
  const reg_t lhs = vm_read(); \
  const reg_t dest = vm_read(); \
  if(reg[lhs] op 0)                     \
    pc = dest;                           \
  DISPATCH();                           \
} while(false)

#define BRANCH_CMP(op) do               \
{                                       \
  const reg_t lhs = vm_read(); \
  const reg_t rhs = vm_read(); \
  const reg_t dest = vm_read(); \
  if(!(reg[lhs] op reg[rhs]))           \
    pc = dest;                           \
  DISPATCH();                           \
} while(false)

void dsvm_run(reg_t *data) {
  reg_t _reg[RVM_SIZE];
  Frame frames[RVM_SIZE];

  reg_t *reg = _reg;
  uint32_t nframe = 0;
  uint32_t pc = 0;

  static const void *dispatch[dsop_max] = {
    &&_dsop_imm,

    &&_dsop_add_imm, &&_dsop_sub_imm, &&_dsop_mul_imm,
    &&_dsop_div_imm, &&_dsop_mod_imm,

    &&_dsop_add, &&_dsop_sub, &&_dsop_mul,
    &&_dsop_div, &&_dsop_mod,

    &&_dsop_eq, &&_dsop_ne, &&_dsop_land, &&_dsop_lor,

    &&_dsop_gt, &&_dsop_ge, &&_dsop_lt, &&_dsop_le,

    &&_dsop_band, &&_dsop_bor, &&_dsop_bxor,
    &&_dsop_blshift, &&_dsop_brshift,

    &&_dsop_inc, &&_dsop_dec,
    &&_dsop_neg, &&_dsop_not, &&_dsop_cmp,

    &&_dsop_call, &&_dsop_return,

    &&_dsop_jump,
    &&_dsop_eq_branch, &&_dsop_ne_branch,
    &&_dsop_gt_branch, &&_dsop_ge_branch,
    &&_dsop_lt_branch, &&_dsop_le_branch,

    &&_dsop_end,
  };
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

  _dsop_eq:
  BINARY(==);
  _dsop_ne:
  BINARY(!=);
  _dsop_land:
  BINARY(&&);
  _dsop_lor:
  BINARY(||);

  _dsop_gt:
  BINARY(>);
  _dsop_ge:
  BINARY(>=);
  _dsop_lt:
  BINARY(<);
  _dsop_le:
  BINARY(<=);

  _dsop_inc:
  UNARY(++);
  _dsop_dec:
  UNARY(--);
  _dsop_neg:
  UNARY(-);
  _dsop_not:
  UNARY(!);
  _dsop_cmp:
  UNARY(~);

  _dsop_call:
{
  reg_t *const _data = (reg_t*)vm_read();
  const reg_t offset = vm_read();
  frames[nframe++] = (Frame){ .offset = reg - _reg, .data = data, .pc = pc };
  reg += offset;
  pc = 0;
  data = _data;
  goto *dispatch[vm_read()];
}
  _dsop_return:
{
  nframe--;
  data = frames[nframe].data;
  reg = _reg + frames[nframe].offset;
  pc = frames[nframe].pc;
  DISPATCH();
}

  _dsop_jump:
{
  const reg_t dest = vm_read();
  pc = dest;
  DISPATCH();
}
  _dsop_eq_branch:
  BRANCH_CMP(==);
  _dsop_ne_branch:
  BRANCH_CMP(!=);
  _dsop_gt_branch:
  BRANCH_CMP(>);
  _dsop_ge_branch:
  BRANCH_CMP(>=);
  _dsop_lt_branch:
  BRANCH_CMP(<);
  _dsop_le_branch:
  BRANCH_CMP(<=);

  _dsop_end:
  printf("result %lu\n", reg[0]);
}

reg_t *dscode_start(void) {
  return _code + _code_count;
}

reg_t *code_alloc(const ds_opcode op, const unsigned int n) {
  reg_t *const code = _code + _code_count;
  *code = op;
  _code_count += n + 1;
  return code;
}
