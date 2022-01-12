#include "ds.h"
#include "dsas.h"
#include "dstb.h"

typedef struct {
  char  *name;
  TB_Function *code;
  TB_DataType ret_type;
  uint32_t narg;
} Fun;

#define FUN_SIZE 2
static Fun fun_data[FUN_SIZE];
static uint32_t fun_count;

#define LABEL_SIZE 2
TB_Label label_data[LABEL_SIZE];

#define VALUE_SIZE 16
TB_Register value_data[VALUE_SIZE];

typedef struct DsTB {
  Fun *curr;
  TB_Module *module;
} DsTB;

static inline void set(const TB_Register reg, const uint32_t n) {
  value_data[n] = reg;
}
#define PARAM_SIZE 8
TB_DataType param_data[PARAM_SIZE];

static TB_Register get_reg(DsTB *const dstb, const uint32_t reg) {
  if(reg < dstb->curr->narg && !value_data[reg]) {
    set(tb_inst_param(dstb->curr->code, reg), reg);
  }  return value_data[reg];
}

#define NOT_FLOAT(code, reg)  !TB_IS_FLOAT_TYPE(tb_node_get_data_type(code, reg).type)

static TB_Register tb_not(TB_Function *code, const TB_Register reg) {
return /*  const TB_Register zero = */TB_IS_INTEGER_TYPE(tb_node_get_data_type(code, reg).type)
    ? tb_inst_sint(code, TB_TYPE_I64, 0)
    : tb_inst_float(code, TB_TYPE_F32, 0);
//  return tb_inst_cmp_ne(code, reg, zero);
}

static TB_Register get_binary(DsTB *const dstb, const DsAsStmt *stmt, TB_Register rhs) {
  TB_Function *code = dstb->curr->code;
  const TB_Register lhs = get_reg(dstb, stmt->num0);
  switch(stmt->op) {
    case dsop_add:
      if(TB_IS_INTEGER_TYPE(tb_node_get_data_type(code, lhs).type))
        return tb_inst_add(code, value_data[5], value_data[8], TB_ASSUME_NUW);
      return tb_inst_fadd(code, lhs, rhs);
    case dsop_sub:
      if(TB_IS_INTEGER_TYPE(tb_node_get_data_type(code, lhs).type))
        return tb_inst_sub(code, lhs, rhs, TB_ASSUME_NUW);
      return tb_inst_fsub(code, lhs, rhs);
    case dsop_mul:
      if(TB_IS_INTEGER_TYPE(tb_node_get_data_type(code, lhs).type))
        return tb_inst_mul(code, lhs, rhs, TB_ASSUME_NUW);
      return tb_inst_fmul(code, lhs, rhs);
    case dsop_div:
      if(TB_IS_INTEGER_TYPE(tb_node_get_data_type(code, lhs).type))
        return tb_inst_div(code, lhs, rhs, TB_ASSUME_NUW);
      return tb_inst_fdiv(code, lhs, rhs);
    case dsop_mod:
      return tb_inst_mod(code, lhs, rhs, TB_ASSUME_NUW);

    case dsop_eq:
      return tb_inst_cmp_eq(code, lhs, rhs);
    case dsop_ne:
      return tb_inst_cmp_ne(code, lhs, rhs);
    case dsop_lt:
      if(TB_IS_INTEGER_TYPE(tb_node_get_data_type(code, lhs).type))
        return tb_inst_cmp_ilt(code, lhs, rhs, TB_ASSUME_NUW);
      return tb_inst_cmp_flt(code, lhs, rhs);
    case dsop_le:
      if(TB_IS_INTEGER_TYPE(tb_node_get_data_type(code, lhs).type))
        return tb_inst_cmp_ile(code, lhs, rhs, TB_ASSUME_NUW);
      return tb_inst_cmp_fle(code, lhs, rhs);
    case dsop_gt:
      if(TB_IS_INTEGER_TYPE(tb_node_get_data_type(code, lhs).type))
        return tb_inst_cmp_igt(code, lhs, rhs, TB_ASSUME_NUW);
      return tb_inst_cmp_fgt(code, lhs, rhs);
    case dsop_ge:
      if(TB_IS_INTEGER_TYPE(tb_node_get_data_type(code, lhs).type))
        return tb_inst_cmp_ige(code, lhs, rhs, TB_ASSUME_NUW);
      return tb_inst_cmp_fge(code, lhs, rhs);

    case dsop_land:
{
  TB_Register ret;
  TB_Label ok0 = tb_inst_new_label_id(code),
           ok1 = tb_inst_new_label_id(code),
           nok = tb_inst_new_label_id(code),
           end = tb_inst_new_label_id(code);

  TB_Register ret0 = tb_not(code, value_data[stmt->num0]);
  tb_inst_if(code, ret0, ok0, nok);

  tb_inst_label(code, ok0);
  TB_Register ret1 = tb_not(code, stmt->num1);
  tb_inst_if(code, ret1, ok1, nok);

  tb_inst_label(code, ok1);
  ret = tb_inst_bool(code, true);
  tb_inst_goto(code, end);

  tb_inst_label(code, nok);
  ret = tb_inst_bool(code, false);
  tb_inst_label(code, end);

  return ret;
}
      break;
    case dsop_lor:
{
  TB_Register ret;
  TB_Label ok0 = tb_inst_new_label_id(code),
           nok = tb_inst_new_label_id(code),
           end = tb_inst_new_label_id(code);

  TB_Register ret0 = tb_not(code, value_data[stmt->num0]);

  tb_inst_if(code, ret0, ok0, nok);

  tb_inst_label(code, nok);
  ret = tb_inst_bool(code, false);
  tb_inst_label(code, end);

  tb_inst_label(code, ok0);
  ret =tb_inst_bool(code, true);
  tb_inst_goto(code, end);

  tb_inst_label(code, end);
  return ret;
}
  break;
    case dsop_band:
      return tb_inst_and(code, lhs, rhs);
  case dsop_bor:
      return tb_inst_or(code, lhs, rhs);
  case dsop_bxor:
      return tb_inst_xor(code, lhs, rhs);
  case dsop_shl:
      return tb_inst_shl(code, lhs, rhs, TB_ASSUME_NUW);
  case dsop_shr:
      return tb_inst_shr(code, lhs, rhs);
  default: exit(12);
  }
}

static void emit_binary(DsTB *const dstb, const DsAsStmt *stmt) {
  set(get_binary(dstb, stmt, get_reg(dstb, stmt->num1)), stmt->dest);
}
/*
static TB_Register get_unary(const ds_opcode op, const TB_Register val) {
//  if(op == dsop_not) return tb_inst_not(code, val);
//  if(op == dsop_cmp) return tb_inst_neg(code, val);
  exit(12);
}
*/
static void emit_unary(DsTB *const dstb, const DsAsStmt *stmt) {
//  const TB_Register reg = get_unary(stmt->op, get_reg(stmt->num0));
//  set(reg, stmt->dest);
}

static void emit_imm(DsTB *const dstb, const DsAsStmt *stmt) {
  const TB_Register reg = tb_inst_sint(dstb->curr->code, TB_TYPE_I64, stmt->num0);
  set(reg, stmt->dest);
}

/*
static void dsjit_emit_immf(const DsAsStmt *stmt) {
  const TB_Register reg = tb_inst_float(code, TB_TYPE_F32 , stmt->fnum0);
  set(reg, stmt->dest);
}
*/

static void emit_call(DsTB *const dstb, const DsAsStmt *stmt) {
  for(uint32_t i =0; i < fun_count; i++) {
    if(stmt->name == fun_data[i].name) {
      Fun fun = fun_data[i];
      set(tb_inst_call(dstb->curr->code, fun.ret_type, fun.code, fun.narg, (TB_Register[]){value_data[stmt->num1]}), stmt->dest);
      return;
   }
  }
  exit(33);
}

static void emit_return(DsTB *const dstb, const DsAsStmt *stmt) {
  if(!stmt->num0)
    tb_inst_ret(dstb->curr->code, get_reg(dstb, stmt->dest));
  else
    tb_inst_ret(dstb->curr->code, TB_NULL_REG);
}

static TB_DataType get_type(const char c) {
  if(c == 'v') return TB_TYPE_VOID;
  if(c == 'i') return TB_TYPE_I64;
  if(c == 'f') return TB_TYPE_F32;
  exit(34);
}

static void emit_label(DsTB *const dstb, const DsAsStmt *stmt) {
  tb_inst_label(dstb->curr->code, label_data[stmt->dest]);
}

static void emit_if(DsTB *const dstb, const DsAsStmt *stmt) {
  tb_inst_if(dstb->curr->code, get_reg(dstb, stmt->num0), label_data[stmt->num1], label_data[stmt->dest]);
}

static void emit_goto(DsTB *const dstb, const DsAsStmt *stmt) {
  tb_inst_goto(dstb->curr->code, label_data[stmt->dest]);
}

typedef void (*dsc_t)(DsTB *const dstb, const DsAsStmt*);

static const dsc_t functions[dsas_function] = {
  emit_binary,
  emit_unary,
  emit_imm,
//  emit_immf,
  emit_label,
  emit_if,
  emit_goto,
  emit_call,
  emit_return,
};

ANN static void compile(DsTB *const dstb, DsAsStmt *stmts, uint32_t start, uint32_t end) {
  for(size_t j = start; j < end; j++) {
    const DsAsStmt stmt = stmts[j];
    functions[stmt.type](dstb, &stmt);
  }
  tb_module_compile_func(dstb->module, dstb->curr->code);
}

static void first_pass(DsTB *const dstb, DsAs *dsas) {
  uint32_t start = 0;
  Fun *fun = NULL;
  for(size_t i = 0; i < dsas->n; i++) {
    const DsAsStmt stmt = dsas->stmts[i];
    if(stmt.type == dsas_function) {
      fun = &fun_data[fun_count];
      fun_data[fun_count].name = stmt.name;
      if(start)
        compile(dstb, dsas->stmts, start, i);
    } else if(stmt.type == dsas_arg) {
      const TB_DataType t = get_type(*stmt.name);
      if(stmt.num1)
        param_data[fun->narg++]  = t;
      else {
        TB_FunctionPrototype* proto = tb_prototype_create(dstb->module, TB_STDCALL, t, fun->narg, false);
        tb_prototype_add_params(proto, fun->narg, param_data);
        dstb->curr = fun;
        fun->code =
          tb_prototype_build(dstb->module, proto, fun->name, TB_LINKAGE_PUBLIC);
        fun->ret_type = t;
        fun_count++;
        start = i + 1;
      }
    } else if(stmt.type == dsas_label)
      label_data[stmt.dest] = tb_inst_new_label_id(fun->code);
  }
  compile(dstb, dsas->stmts, start, dsas->n);
}

TB_Function *dstb_compile(TB_Module* module, DsAs *dsas) {
  DsTB dstb = { .module = module };
  first_pass(&dstb, dsas);
  return dstb.curr->code;
}
