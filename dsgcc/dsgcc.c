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

ANN static gcc_jit_type* get_type(DsGcc *const dsgcc, const char c) {
  return gcc_jit_context_get_type(dsgcc->ctx, _get_type(c));
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
  if(op == dsop_land) return GCC_JIT_BINARY_OP_LOGICAL_AND;
  if(op == dsop_lor) return GCC_JIT_BINARY_OP_LOGICAL_OR;
  if(op == dsop_shl) return GCC_JIT_BINARY_OP_LSHIFT;
  if(op == dsop_shr) return GCC_JIT_BINARY_OP_RSHIFT;
  exit(op);
}

static inline enum gcc_jit_comparison get_comp(const ds_opcode op) {
  return GCC_JIT_COMPARISON_EQ + op - dsop_eq;
}
static void actual_emit_binary(DsGcc *const dsgcc, const DsAsStmt *stmt, gcc_jit_rvalue *rhs) {
  if(stmt->op < dsop_lt || stmt->op > dsop_ge) {
    dsgcc->value_data[stmt->dest] = gcc_jit_context_new_binary_op (dsgcc->ctx,
        LOC, get_binary(stmt->op), gcc_jit_rvalue_get_type(rhs), dsgcc->value_data[stmt->num0], rhs);
  } else {
    dsgcc->value_data[stmt->dest] = gcc_jit_context_new_comparison (dsgcc->ctx,
        LOC, get_comp(stmt->op), dsgcc->value_data[stmt->num0], rhs);
  }
}

ANN static void emit_binary(DsGcc *const dsgcc, const DsAsStmt *stmt) {
  actual_emit_binary(dsgcc, stmt, dsgcc->value_data[stmt->num1]);
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

ANN static void emit_unary(DsGcc *const dsgcc, const DsAsStmt *stmt) {
  gcc_jit_rvalue *const val = dsgcc->value_data[stmt->num0];
  dsgcc->value_data[stmt->dest] = gcc_jit_context_new_unary_op (dsgcc->ctx,
        LOC, get_unary(stmt->op), gcc_jit_rvalue_get_type(val), val);
}

ANN static void emit_imm(DsGcc *const dsgcc, const DsAsStmt *stmt) {
  dsgcc->value_data[stmt->dest] = gcc_jit_context_new_rvalue_from_long (dsgcc->ctx,
              get_type(dsgcc, 'i'), stmt->num0);
}

ANN static void emit_call(DsGcc *const dsgcc, const DsAsStmt *stmt) {
  //for(uint32_t i =0; i < dsgcc->nfun; i++) {
  Fun *data = dsgcc->fun_data;
  while(data->name) {
//    const Fun fun = *dsgcc->curr;
    if(stmt->name == data->name) {
      dsgcc->value_data[stmt->dest] =
        gcc_jit_context_new_call(dsgcc->ctx,
            LOC, data->code, data->narg, dsgcc->value_data + stmt->num1);
      return;
   }
    data++;
  }
  exit(33);
}

ANN static void emit_return(DsGcc *const dsgcc, const DsAsStmt *stmt) {
  if(!stmt->num0) {
    gcc_jit_block_end_with_return (dsgcc->block,
        LOC, dsgcc->value_data[stmt->dest]);
  } else
    gcc_jit_block_end_with_void_return (dsgcc->block, LOC);
  dsgcc->is_terminated = true;
}

ANN static void emit_if(DsGcc *const dsgcc, const DsAsStmt *stmt) {
  dsgcc->is_terminated = true;
  gcc_jit_block_end_with_conditional (dsgcc->block,
      LOC, dsgcc->value_data[stmt->num0], dsgcc->label_data[stmt->num1], dsgcc->label_data[stmt->dest]);
}

ANN static void emit_goto(DsGcc *const dsgcc, const DsAsStmt *stmt) {
  gcc_jit_block_end_with_jump (dsgcc->block, LOC, dsgcc->label_data[stmt->dest]);
}

ANN static void emit_label(DsGcc *const dsgcc, const DsAsStmt *stmt) {
  gcc_jit_block *next = dsgcc->label_data[stmt->dest];
  if(!dsgcc->is_terminated)
    gcc_jit_block_end_with_jump (dsgcc->block, LOC, next);
  dsgcc->block = dsgcc->label_data[stmt->dest];
  dsgcc->is_terminated = 0;
}

typedef void (*dsc_t)(DsGcc *const dsgcc, const DsAsStmt*);

static const dsc_t functions[dsas_function] = {
  emit_binary,
  emit_unary,
  emit_imm,
  emit_label,
  emit_if,
  emit_goto,
  emit_call,
  emit_return
};

ANN void dsgcc_compile(DsGcc *const dsgcc, DsAs *dsas) {
  char *name = NULL;
  uint32_t narg = 0;
  uint32_t start = 0;
  Fun fun = {};
  #define PARAM_SIZE 16
  gcc_jit_param *param_data[PARAM_SIZE];
  for(uint32_t i = 0; i < dsas->n; i++) {
    const DsAsStmt stmt = dsas->stmts[i];
    if(stmt.type == dsas_function) {
      name = stmt.name;
      for(size_t j = start; j < i; j++) {
        const DsAsStmt stmt = dsas->stmts[j];
        functions[stmt.type](dsgcc, &stmt);
      }
    } else if(stmt.type == dsas_arg) {
      gcc_jit_type *const t = get_type(dsgcc, *stmt.name);
      if(stmt.num1) {
        gcc_jit_param *const param = param_data[narg] = gcc_jit_context_new_param (dsgcc->ctx, NULL, t,  "arg");
        dsgcc->value_data[narg] = gcc_jit_param_as_rvalue(param);
        narg++;
      } else {
        gcc_jit_function *const code = gcc_jit_context_new_function (dsgcc->ctx, NULL, GCC_JIT_FUNCTION_EXPORTED,
            t, name, narg, param_data, 0);
        fun = dsgcc->fun_data[dsgcc->nfun++] = (Fun) { .name=name, .code=code, .narg=narg };
        narg = 0;
          dsgcc->block = gcc_jit_function_new_block (fun.code, "start");
        start = i + 1;
        dsgcc->curr = &fun;
      }
    } else if(stmt.type == dsas_label) {
      char c[16];
      sprintf(c, "L%u", i);
      dsgcc->label_data[stmt.dest] = gcc_jit_function_new_block (dsgcc->fun_data[dsgcc->nfun-1].code, c);
    }
  }
  for(size_t j = start; j < dsas->n; j++) {
    const DsAsStmt stmt = dsas->stmts[j];
    functions[stmt.type](dsgcc, &stmt);
  }
}
