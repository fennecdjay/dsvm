#include <libgccjit.h>
#include "ds.h"
#include "dsas.h"
#include "dsgcc.h"

ANN static enum gcc_jit_types _get_type(const char c) {
  if(c == 'v') return GCC_JIT_TYPE_VOID;
  if(c == 'i') return GCC_JIT_TYPE_LONG;
  if(c == 'f') return GCC_JIT_TYPE_FLOAT;
  exit(12);
}

ANN static gcc_jit_type* get_type(DsJit *const dsjit, const char c) {
  return gcc_jit_context_get_type(dsjit->ctx, _get_type(c));
}

static enum gcc_jit_binary_op get_binary(const ds_opcode op) {
  if(op == dsop_add /*|| op == dsop_addf*/) return GCC_JIT_BINARY_OP_PLUS;
  if(op == dsop_sub /*|| op == dsop_subf*/) return GCC_JIT_BINARY_OP_MINUS;
  if(op == dsop_mul /*|| op == dsop_mulf*/) return GCC_JIT_BINARY_OP_MULT;
  if(op == dsop_div /*|| op == dsop_divf*/) return GCC_JIT_BINARY_OP_DIVIDE;
  if(op == dsop_mod) return GCC_JIT_BINARY_OP_MODULO;
  if(op == dsop_band) return GCC_JIT_BINARY_OP_BITWISE_AND;
  if(op == dsop_bor) return GCC_JIT_BINARY_OP_BITWISE_OR;
  if(op == dsop_bxor) return GCC_JIT_BINARY_OP_BITWISE_XOR;
//  if(op == dsop_land) return GCC_JIT_BINARY_OP_LOGICAL_AND;
//  if(op == dsop_lor) return GCC_JIT_BINARY_OP_LOGICAL_OR;
  if(op == dsop_shl) return GCC_JIT_BINARY_OP_LSHIFT;
  if(op == dsop_shr) return GCC_JIT_BINARY_OP_RSHIFT;
  exit(op);
}

static inline enum gcc_jit_comparison get_comp(const ds_opcode op) {
//  return GCC_JIT_COMPARISON_EQ + op - dsop_eq;
  return GCC_JIT_COMPARISON_LT + op - dsop_lt;
}
static void actual_emit_binary(DsJit *const dsjit, const DscStmt *stmt, gcc_jit_rvalue *rhs) {
  if(stmt->op < dsop_lt || stmt->op > dsop_ge) {
    dsjit->value_data[stmt->dest] = gcc_jit_context_new_binary_op (dsjit->ctx,
        LOC, get_binary(stmt->op), gcc_jit_rvalue_get_type(rhs), dsjit->value_data[stmt->num0], rhs);
  } else {
    dsjit->value_data[stmt->dest] = gcc_jit_context_new_comparison (dsjit->ctx,
        LOC, get_comp(stmt->op), dsjit->value_data[stmt->num0], rhs);
  }
}

ANN static void dsjit_emit_binary(DsJit *const dsjit, const DscStmt *stmt) {
  actual_emit_binary(dsjit, stmt, dsjit->value_data[stmt->num1]);
}

static enum gcc_jit_unary_op  get_unary(const ds_opcode op) {
//  if(op == dsop_inc) exit(3)
//  if(op == dsop_dec) exit(4);
//  if(op == dsop_mov) exit(5); // ??
//  if(op == dsop_neg) return GCC_JIT_UNARY_OP_MINUS;
//  if(op == dsop_not) return GCC_JIT_UNARY_OP_LOGICAL_NEGATE;
//  if(op == dsop_cmp) return GCC_JIT_UNARY_OP_BITWISE_NEGATE;
  //  if(op == dsop_add)
  return GCC_JIT_UNARY_OP_ABS;
}

ANN static void dsjit_emit_unary(DsJit *const dsjit, const DscStmt *stmt) {
  gcc_jit_rvalue *const val = dsjit->value_data[stmt->num0];
  dsjit->value_data[stmt->dest] = gcc_jit_context_new_unary_op (dsjit->ctx,
        LOC, get_unary(stmt->op), gcc_jit_rvalue_get_type(val), val);
}

ANN static void dsjit_emit_imm(DsJit *const dsjit, const DscStmt *stmt) {
  dsjit->value_data[stmt->dest] = gcc_jit_context_new_rvalue_from_long (dsjit->ctx,
              get_type(dsjit, 'i'), stmt->num0);
}

ANN static void dsjit_emit_call(DsJit *const dsjit, const DscStmt *stmt) {
  //for(uint32_t i =0; i < dsjit->nfun; i++) {
  Fun *data = dsjit->fun_data;
  while(data->name) {
//    const Fun fun = *dsjit->curr;
    if(stmt->name == data->name) {
      dsjit->value_data[stmt->dest] =
        gcc_jit_context_new_call(dsjit->ctx,
            LOC, data->code, data->narg, dsjit->value_data + stmt->num1);
      return;
   }
    data++;
  }
  exit(33);
}

ANN static void dsjit_emit_return(DsJit *const dsjit, const DscStmt *stmt) {
  if(!stmt->num0) {
    gcc_jit_block_end_with_return (dsjit->block,
        LOC, dsjit->value_data[stmt->dest]);
  } else
    gcc_jit_block_end_with_void_return (dsjit->block, LOC);
  dsjit->is_terminated = true;
}

ANN static void dsjit_emit_if(DsJit *const dsjit, const DscStmt *stmt) {
  dsjit->is_terminated = true;
  gcc_jit_block_end_with_conditional (dsjit->block,
      LOC, dsjit->value_data[stmt->num0], dsjit->label_data[stmt->num1], dsjit->label_data[stmt->dest]);
}

ANN static void dsjit_emit_goto(DsJit *const dsjit, const DscStmt *stmt) {
  gcc_jit_block_end_with_jump (dsjit->block, LOC, dsjit->label_data[stmt->dest]);
}

ANN static void dsjit_emit_label(DsJit *const dsjit, const DscStmt *stmt) {
  gcc_jit_block *next = dsjit->label_data[stmt->dest];
  if(!dsjit->is_terminated)
    gcc_jit_block_end_with_jump (dsjit->block, LOC, next);
  dsjit->block = dsjit->label_data[stmt->dest];
  dsjit->is_terminated = 0;
}

typedef void (*dsc_t)(DsJit *const dsjit, const DscStmt*);

static const dsc_t functions[dsc_function] = {
  dsjit_emit_binary,
  dsjit_emit_unary,
  dsjit_emit_imm,
  dsjit_emit_label,
  dsjit_emit_if,
  dsjit_emit_goto,
  dsjit_emit_call,
  dsjit_emit_return
};

ANN void dsgcc_compile(DsJit *const dsjit, DsScanner *ds) {
  char *name = NULL;
  uint32_t narg = 0;
  uint32_t start = 0;
  Fun fun = {};
  #define PARAM_SIZE 16
  gcc_jit_param *param_data[PARAM_SIZE];
  for(size_t i = 0; i < ds->n; i++) {
    const DscStmt stmt = ds->stmts[i];
    if(stmt.type == dsc_function) {
      name = stmt.name;
  for(size_t j = start; j < i; j++) {
    const DscStmt stmt = ds->stmts[j];
    functions[stmt.type](dsjit, &stmt);
  }
}
    else if(stmt.type == dsc_arg) {
      gcc_jit_type *const t = get_type(dsjit, *stmt.name);
      if(stmt.num1) {
        gcc_jit_param *const param = param_data[narg] = gcc_jit_context_new_param (dsjit->ctx, NULL, t,  "arg");
        dsjit->value_data[narg] = gcc_jit_param_as_rvalue(param);
        narg++;
      } else {
        gcc_jit_function *const code = gcc_jit_context_new_function (dsjit->ctx, NULL, GCC_JIT_FUNCTION_EXPORTED,
            t, name, narg, param_data, 0);
        fun = dsjit->fun_data[dsjit->nfun++] = (Fun) { .name=name, .code=code, .narg=narg };
        narg = 0;
          dsjit->block = gcc_jit_function_new_block (fun.code, "start");
        start = i + 1;
        dsjit->curr = &fun;
      }
    } else if(stmt.type == dsc_label) {
      char c[16];
      sprintf(c, "L%u", i);
      dsjit->label_data[stmt.dest] = gcc_jit_function_new_block (dsjit->fun_data[dsjit->nfun-1].code, c);
    }
  }
  for(size_t j = start; j < ds->n; j++) {
    const DscStmt stmt = ds->stmts[j];
    functions[stmt.type](dsjit, &stmt);
  }
}
