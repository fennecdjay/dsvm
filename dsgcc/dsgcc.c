#include <libgccjit.h>
#include "ds.h"
#include "dsas.h"
#include "dsc.h"
#include "dsgcc.h"

ANN static enum gcc_jit_types _get_type(const char c) {
  if(c == 'v') return GCC_JIT_TYPE_VOID;
  if(c == 'i') return GCC_JIT_TYPE_LONG;
  if(c == 'f') return GCC_JIT_TYPE_FLOAT;
  printf("unknown type %c\n", c);
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
static gcc_jit_rvalue *actual_emit_binary(DsGcc *const dsgcc, const DsAsStmt *stmt, gcc_jit_rvalue *rhs) {
  const ds_opcode op = stmt->op;
  if(op < dsop_eq || op > dsop_ge)
    return gcc_jit_context_new_binary_op (dsgcc->ctx,
        LOC, get_binary(stmt->op), gcc_jit_rvalue_get_type(rhs), dsgcc->value_data[stmt->num0], rhs);
  return gcc_jit_context_new_comparison (dsgcc->ctx,
      LOC, get_comp(stmt->op), dsgcc->value_data[stmt->num0], rhs);
}

ANN static void emit_binary(DsGcc *const dsgcc, const DsAsStmt *stmt) {
  if(stmt->op < dsop_add_imm)
    dsgcc->value_data[stmt->dest] = actual_emit_binary(dsgcc, stmt, dsgcc->value_data[stmt->num1]);
  else {
    gcc_jit_rvalue *rhs = gcc_jit_context_new_rvalue_from_long (dsgcc->ctx, get_type(dsgcc, 'i'), stmt->num1);
    DsAsStmt tmp = { .op = stmt->op - dsop_add_imm + dsop_add, .num0 = stmt->num0, .dest = stmt->dest };
    dsgcc->value_data[stmt->dest] = actual_emit_binary(dsgcc, &tmp, rhs);
  }
}

ANN static gcc_jit_rvalue *unary(DsGcc *const dsgcc, enum gcc_jit_unary_op op, const reg_t idx) {
  gcc_jit_rvalue *const val = dsgcc->value_data[idx];
  return gcc_jit_context_new_unary_op (dsgcc->ctx, LOC, op, gcc_jit_rvalue_get_type(val), val);
}

ANN static void emit_unary(DsGcc *const dsgcc, const DsAsStmt *stmt) {
  const ds_opcode op = stmt->op;
  if(op == dsop_sub)      dsgcc->value_data[stmt->dest] = unary(dsgcc, GCC_JIT_UNARY_OP_MINUS, stmt->num0);
  else if(op == dsop_not) dsgcc->value_data[stmt->dest] = unary(dsgcc, GCC_JIT_UNARY_OP_LOGICAL_NEGATE, stmt->num0);
  else if(op == dsop_cmp) dsgcc->value_data[stmt->dest] = unary(dsgcc, GCC_JIT_UNARY_OP_BITWISE_NEGATE, stmt->num0);
  else if(op == dsop_add) dsgcc->value_data[stmt->dest] = unary(dsgcc, GCC_JIT_UNARY_OP_ABS, stmt->num0);
  else if(op == dsop_mov) gcc_jit_block_add_assignment (dsgcc->block, LOC, dsgcc->lvalue_data[stmt->dest], dsgcc->value_data[stmt->num0]);
  else if(op == dsop_mul) dsgcc->value_data[stmt->dest] = gcc_jit_lvalue_as_rvalue(
          gcc_jit_rvalue_dereference(dsgcc->value_data[stmt->num0], LOC));
  else if(op == dsop_mul) dsgcc->value_data[stmt->dest] = gcc_jit_lvalue_get_address(dsgcc->lvalue_data[stmt->num0], LOC);
  exit(14);
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
  if(stmt->op == dsop_imm) {
    gcc_jit_block_end_with_conditional (dsgcc->block,
      LOC, dsgcc->value_data[stmt->num0], dsgcc->label_data[stmt->num1], dsgcc->label_data[stmt->dest]);
  } else {
    gcc_jit_block *next = gcc_jit_function_new_block(dsgcc->curr->code, NULL);
    gcc_jit_rvalue *rhs = stmt->op < dsop_if_add_imm
      ? dsgcc->value_data[stmt->num1]
      : gcc_jit_context_new_rvalue_from_long (dsgcc->ctx, get_type(dsgcc, 'i'), stmt->num1);
    const ds_opcode op = stmt->op < dsop_if_add_imm
      ? stmt->op - dsop_if_add + dsop_add
      : stmt->op - dsop_if_add_imm + dsop_add;
    DsAsStmt tmp = { .op = op, .num0 = stmt->num0 };
    gcc_jit_rvalue *cond = actual_emit_binary(dsgcc, &tmp, rhs);
    gcc_jit_type *type = gcc_jit_rvalue_get_type (cond),
                 *tbool = gcc_jit_context_get_type(dsgcc->ctx, GCC_JIT_TYPE_BOOL);
    if(type != tbool)
      cond = gcc_jit_context_new_cast (dsgcc->ctx, LOC, cond, tbool);
    gcc_jit_block_end_with_conditional (dsgcc->block,
      LOC, cond, next, dsgcc->label_data[stmt->dest]);
    dsgcc->block = next;
  }
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

typedef void (*dsgcc_t)(DsGcc *const dsgcc, const DsAsStmt*);

static const dsgcc_t functions[dsas_function] = {
  emit_binary,
  emit_unary,
  emit_imm,
  emit_label,
  emit_if,
  emit_goto,
  emit_call,
  emit_return
};

#define PARAM_SIZE 16

ANN static void mk_trampoline(const DsGcc *dsgcc, Fun *fun, const char *name) {
  gcc_jit_context *const ctx = dsgcc->ctx;
  gcc_jit_param *const param = gcc_jit_context_new_param (ctx, LOC, gcc_jit_type_get_pointer (gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_LONG)), "reg");
  gcc_jit_param *params[1] = { param };
  gcc_jit_function *const code = gcc_jit_context_new_function(ctx, LOC, GCC_JIT_FUNCTION_EXPORTED,
    fun->ret_type, name, 1, params, 0);
  gcc_jit_block *block = gcc_jit_function_new_block (code, NULL);
  gcc_jit_rvalue *reg = gcc_jit_param_as_rvalue(param);
  gcc_jit_rvalue *args[PARAM_SIZE];
  for(uint32_t i = 0; i < fun->narg; i++) {
    gcc_jit_rvalue *const index = gcc_jit_context_new_rvalue_from_int(ctx, gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_UNSIGNED_INT), i);
    args[i] = gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(ctx, LOC, reg, index));
  }
  gcc_jit_rvalue *ret = gcc_jit_context_new_call(ctx, LOC, fun->code, fun->narg, args);
  gcc_jit_block_end_with_return(block, LOC, ret);
}

ANN void dsgcc_compile(DsGcc *const dsgcc, const DsAs *dsas) {
  char *name = NULL;
  uint32_t narg = 0;
  uint32_t start = 0;
  Fun fun = {};
  gcc_jit_param *param_data[PARAM_SIZE];
  for(uint32_t i = 0; i < dsas->n; i++) {
    const DsAsStmt stmt = dsas->stmts[i];
    if(stmt.type == dsas_function) {
      name = stmt.name;
      for(size_t j = start; j < i; j++) {
        const DsAsStmt stmt = dsas->stmts[j];
        functions[stmt.type](dsgcc, &stmt);
      }
      if(dsgcc->dsc && dsgcc->curr) {
        mk_trampoline(dsgcc, dsgcc->curr, dsgcc->dsc->fun_data[dsgcc->nfun - 1].trampoline_name);
        if(dsgcc->nfun == dsgcc->dsc->fun_count - 1) break;
      }
    } else if(stmt.type == dsas_arg) {
      gcc_jit_type *const t = get_type(dsgcc, *stmt.name);
      if(stmt.num1) {
        char c[256];
        sprintf(c, "arg%u", narg);
        gcc_jit_param *const param = param_data[narg] = gcc_jit_context_new_param (dsgcc->ctx, LOC, t, c);
        dsgcc->value_data[narg] = gcc_jit_param_as_rvalue(param);
        narg++;
      } else {
        gcc_jit_function *const code = gcc_jit_context_new_function (dsgcc->ctx, LOC, GCC_JIT_FUNCTION_EXPORTED,
            t, name, narg, param_data, 0);
        fun = dsgcc->fun_data[dsgcc->nfun++] = (Fun) { .name=name, .code=code, .narg=narg, .ret_type = t };
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
  if(!dsgcc->dsc) {
    for(size_t j = start; j < dsas->n; j++) {
      const DsAsStmt stmt = dsas->stmts[j];
      functions[stmt.type](dsgcc, &stmt);
    }
  }
}

#ifndef DS_NOJIT
ANN int gcc_launch_jit(void *data) {
  const Jitter *jitter = (Jitter*)data;
  Dsc *const dsc = jitter->dsc;
  const DsAs *dsas = jitter->dsas;
  gcc_jit_context *ctx = gcc_jit_context_acquire();
  gcc_jit_context_set_int_option (ctx, GCC_JIT_INT_OPTION_OPTIMIZATION_LEVEL, 3);
  DsGcc dsgcc = { .ctx = ctx, .dsc = dsc };
  dsgcc_compile(&dsgcc, dsas);
  gcc_jit_result *result = gcc_jit_context_compile (ctx);
  for(size_t i = 0; i < dsc->fun_count - 1; i++) {
    DscFun *fun = dsgcc.curr = &dsc->fun_data[i];
    *(void**)(fun->code + sizeof(void*)) = gcc_jit_result_get_code (result, fun->trampoline_name);
  }
  thrd_exit(EXIT_SUCCESS);
}
#endif
