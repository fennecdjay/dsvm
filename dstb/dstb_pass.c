#include <tb/tb.h>
#include "ds.h"
#include "dsc.h"
#include "generated/dsc_parser.h"
#include "generated/dsc_lexer.h"

typedef struct {
  char  *name;
  TB_Function *code;
  TB_DataType ret_type;
  uint32_t narg;
} Fun;

static TB_Function *code;

#define FUN_SIZE 256
static Fun fun_data[FUN_SIZE];
static uint32_t fun_count;

#define LABEL_SIZE 64
TB_Label label_data[LABEL_SIZE];

#define LABEL_SIZE 64
TB_Label jump_data[LABEL_SIZE];
static uint32_t jump_count;

#define VALUE_SIZE 256
TB_Register value_data[VALUE_SIZE];

static inline void set(const TB_Register reg, const uint32_t n) {
  value_data[n] = reg;
}
#define PARAM_SIZE 16
TB_DataType param_data[PARAM_SIZE];

static TB_Register get_reg(const uint32_t reg) {
  if(reg < fun_data[fun_count - 1].narg && !value_data[reg]) {
    return value_data[reg] = tb_inst_param(code, reg);
  }
  return value_data[reg];
}

static TB_Register get_binary(const DsStmt *stmt, TB_Register rhs) {
  const TB_Register lhs = get_reg(stmt->num0);
  switch(stmt->op) {
    case dsop_add:
      if(TB_IS_INTEGER_TYPE(lhs))
        return tb_inst_add(code, lhs, rhs, TB_ASSUME_NUW);
      return tb_inst_fadd(code, lhs, rhs);
    case dsop_sub:
      if(TB_IS_INTEGER_TYPE(lhs))
        return tb_inst_sub(code, lhs, rhs, TB_ASSUME_NUW);
      return tb_inst_fsub(code, lhs, rhs);
    case dsop_mul:
      if(TB_IS_INTEGER_TYPE(lhs))
        return tb_inst_mul(code, lhs, rhs, TB_ASSUME_NUW);
      return tb_inst_fmul(code, lhs, rhs);
    case dsop_div:
      if(TB_IS_INTEGER_TYPE(lhs))
        return tb_inst_div(code, lhs, rhs, TB_ASSUME_NUW);
      return tb_inst_fdiv(code, lhs, rhs);
    case dsop_mod:
      return tb_inst_mod(code, lhs, rhs, TB_ASSUME_NUW);

    case dsop_eq:
      return tb_inst_cmp_eq(code, lhs, rhs);
    case dsop_ne:
      return tb_inst_cmp_ne(code, lhs, rhs);
    case dsop_lt:
      if(TB_IS_INTEGER_TYPE(lhs))
        return tb_inst_cmp_ilt(code, lhs, rhs, TB_ASSUME_NUW);
      return tb_inst_cmp_flt(code, lhs, rhs);
    case dsop_le:
      if(TB_IS_INTEGER_TYPE(lhs))
        return tb_inst_cmp_ile(code, lhs, rhs, TB_ASSUME_NUW);
      return tb_inst_cmp_fle(code, lhs, rhs);
    case dsop_gt:
      if(TB_IS_INTEGER_TYPE(lhs))
        return tb_inst_cmp_igt(code, lhs, rhs, TB_ASSUME_NUW);
      return tb_inst_cmp_fgt(code, lhs, rhs);
    case dsop_ge:
      if(TB_IS_INTEGER_TYPE(lhs))
        return tb_inst_cmp_ige(code, lhs, rhs, TB_ASSUME_NUW);
      return tb_inst_cmp_fge(code, lhs, rhs);

    case dsop_land:
    case dsop_lor:

    case dsop_band:
      return tb_inst_and(code, lhs, rhs);
  case dsop_bor:
      return tb_inst_or(code, lhs, rhs);
  case dsop_bxor:
      return tb_inst_xor(code, lhs, rhs);
  case dsop_blshift:
      return tb_inst_shl(code, lhs, rhs, TB_ASSUME_NUW);
  case dsop_brshift:
      return tb_inst_shr(code, lhs, rhs);
  default: exit(12);
  }
}

static void dsjit_emit_binary(const DsStmt *stmt) {
  set(get_binary(stmt, get_reg(stmt->num1)), stmt->dest);
}

static void dsjit_emit_ibinary(const DsStmt *stmt) {
  const TB_Register reg = tb_inst_sint(code, TB_TYPE_I32, stmt->num1);
  set(get_binary(stmt, reg), stmt->dest);
}

static void dsjit_emit_fbinary(const DsStmt *stmt) {
  const TB_Register reg = tb_inst_float(code, TB_TYPE_F32, stmt->num1);
  set(get_binary(stmt, reg), stmt->dest);
}

static TB_Register get_unary(const ds_opcode op, const TB_Register val) {
  if(op == dsop_not) return tb_inst_not(code, val);
  if(op == dsop_cmp) return tb_inst_neg(code, val);
  exit(12);
}

static void dsjit_emit_unary(const DsStmt *stmt) {
  const TB_Register reg = get_unary(stmt->op, get_reg(stmt->num0));
  set(reg, stmt->dest);
}

static void dsjit_emit_imm(const DsStmt *stmt) {
  const TB_Register reg = tb_inst_sint(code, TB_TYPE_I32, stmt->num0);
  set(reg, stmt->dest);
}

static void dsjit_emit_immf(const DsStmt *stmt) {
  const TB_Register reg = tb_inst_float(code, TB_TYPE_F32 , stmt->fnum0);
  set(reg, stmt->dest);
}

static void dsjit_emit_jump(const DsStmt *stmt) {
  if(stmt->op != dsop_jump) {
    DsStmt fake = {
      .op = stmt->op - dsop_eq_jump + dsop_eq,
      .num0 = stmt->num0,
    };
    TB_Register tmp = get_binary(&fake, get_reg(stmt->num1));
    TB_Label if_false = jump_data[jump_count++];
    tb_inst_if(code, tmp, label_data[stmt->dest], if_false);
    tb_inst_label(code, if_false);
  } else tb_inst_goto(code, label_data[stmt->dest]);
}

static void dsjit_emit_call(const DsStmt *stmt) {
  for(uint32_t i =0; i < fun_count; i++) {
    if(stmt->name == fun_data[i].name) {
      Fun fun = fun_data[i];
      set(tb_inst_call(code, fun.ret_type, fun.code, fun.narg, value_data + stmt->num1), stmt->dest);
      return;
   }
  }
  exit(33);
}

static void dsjit_emit_return(const DsStmt *stmt) {
  if(!stmt->num0)
    tb_inst_ret(code, get_reg(stmt->dest));
  else
    tb_inst_ret(code, TB_NULL_REG);
}

static void dsjit_emit_function(const DsStmt *stmt) {
  (void)stmt;
  code = fun_data[fun_count].code;
  fun_count++;
  jump_count = 0;
}

static TB_DataType get_type(const char c) {
  if(c == 'v') return TB_TYPE_VOID;
  if(c == 'i') return TB_TYPE_I32;
  if(c == 'f') return TB_TYPE_F32;
  exit(34);
}

static void dsjit_emit_argument(const DsStmt *stmt) {
  (void)stmt;
}

static void dsjit_emit_label(const DsStmt *stmt) {
  tb_inst_label(code, label_data[stmt->dest]);
}

static void dsjit_emit_if(const DsStmt *stmt) {
  tb_inst_if(code, get_reg(stmt->num0), label_data[stmt->num1], label_data[stmt->dest]);
}

typedef void (*dsc_t)(const DsStmt*);

static void first_pass(TB_Module *module, DsScanner *ds) {
  uint32_t fun_count = 0;
  for(size_t i = 0; i < ds->n; i++) {

    const DsStmt stmt = ds->stmts[i];
    if(stmt.type == dsc_function) {
      fun_data[fun_count].name = stmt.name;
    } else if(stmt.type == dsc_arg) {
      const TB_DataType t = get_type(*stmt.name);
      Fun *fun = &fun_data[fun_count];
      if(stmt.num1)
        param_data[fun->narg++]  = t;
      else {
        TB_FunctionPrototype* proto = tb_prototype_create(module, TB_STDCALL, t, fun->narg, false);

        tb_prototype_add_params(proto, fun->narg, param_data);
        code = fun->code =
          tb_prototype_build(module, proto, fun->name, TB_LINKAGE_PUBLIC);
        fun->ret_type = t;
        fun_count ++;
      }
    } else if(stmt.type == dsc_label) {
      label_data[stmt.dest] = tb_inst_new_label_id(code);
    } //else if(stmt.type == dsc_jump) {
      //jump_data[jump_count++] = tb_inst_new_label_id(code);
 //   }
  }
}

static const dsc_t functions[dsop_max] = {
  dsjit_emit_binary,
  dsjit_emit_ibinary,
  dsjit_emit_fbinary,
  dsjit_emit_unary,
  dsjit_emit_imm,
  dsjit_emit_immf,
  dsjit_emit_jump,
  dsjit_emit_call,
  dsjit_emit_return,
  dsjit_emit_function,
  dsjit_emit_argument,
  dsjit_emit_label,
  dsjit_emit_if
};


TB_Function *dtb_compile(TB_Module* module, DsScanner *ds) {
  first_pass(module, ds);
  for(size_t i = 0; i < ds->n; i++) {
    const DsStmt stmt = ds->stmts[i];
    functions[stmt.type](&stmt);
  }
  return code;
}
