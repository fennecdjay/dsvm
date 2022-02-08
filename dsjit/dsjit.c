#include <jit/jit.h>
#include "ds.h"
#include "dsas.h"
#include "dsc.h"
#include "dsjit.h"

static jit_value_t get(DsJit *const dsjit, const uint32_t idx) {
  if(idx < dsjit->curr->narg) return jit_value_get_param(dsjit->fun, idx);
  return dsjit->value[idx];
}

static jit_value_t binary(DsJit *const dsjit, const DsAsStmt *stmt){
  const jit_value_t lhs = get(dsjit, stmt->num0),
                    rhs = stmt->op < dsop_add_imm
                        ? get(dsjit, stmt->num1)
                        : jit_value_create_long_constant(dsjit->fun, jit_type_long, stmt->num1);
  const ds_opcode op = stmt->op < dsop_add_imm ? stmt->op : stmt->op - dsop_add_imm + dsop_add;
  if(op == dsop_add)
    return jit_insn_add(dsjit->fun, lhs, rhs);
  if(op == dsop_sub)
    return jit_insn_sub(dsjit->fun, lhs, rhs);
  if(op == dsop_mul)
    return jit_insn_mul(dsjit->fun, lhs, rhs);
  if(op == dsop_div)
    return jit_insn_div(dsjit->fun, lhs, rhs);
  if(op == dsop_mod)
    return jit_insn_rem(dsjit->fun, lhs, rhs);
  if(op == dsop_eq)
    return jit_insn_eq(dsjit->fun, lhs, rhs);
  if(op == dsop_ne)
    return jit_insn_ne(dsjit->fun, lhs, rhs);
  if(op == dsop_lt)
    return jit_insn_lt(dsjit->fun, lhs, rhs);
  if(op == dsop_le)
    return jit_insn_le(dsjit->fun, lhs, rhs);
  if(op == dsop_gt)
    return jit_insn_gt(dsjit->fun, lhs, rhs);
  if(op == dsop_ge)
    return jit_insn_ge(dsjit->fun, lhs, rhs);
// implement && and ||
  if(op == dsop_land) {
/*
    jit_label_t rok = jit_label_undefined;
    jit_label_t  ok = jit_label_undefined;
    jit_value_t lok = jit_insn_true(dsjit->fun, lhs);
    jit_insn_branch_if(dsjit->fun, lok, &rok);
    jit_value_t rok = jit_insn_true(dsjit->fun, rhs);
    jit_insn_branch_if(dsjit->fun, rok, &ok);
    jit_value_create_long_constant(dsjit->fun, jit_type_long, 0);
    jit_insn_label(dsjit->fun, &ok);
    jit_value_create_long_constant(dsjit->fun, jit_type_long, 1);
*/
  }
  if(op == dsop_lor) {
 //   jit_value_t jit_value_create(dsjit->fun, jit_type_long)
 //   jit_value_t lok = jit_insn_true(dsjit->fun, lhs);
 //   jit_insn_branch_if_not(dsjit->fun, lok, &end);
 //   jit_value_t rok = jit_insn_true(dsjit->fun, ehs);
 //   jit_value_create_long_constant(dsjit->fun, jit_type_long, 0);
 //   jit_insn_branch_if_not(dsjit->fun, lok, &end);
 //   jit_insn_return(dsjit->fun, one);
 //   jit_value_create_long_constant(dsjit->fun, jit_type_long, 0);
 //   jit_insn_return(dsjit->fun, zero);
// if !lhs goto :end
// if rhs
// return 1
// :end
// return 0
/*
    jit_label_t ok = jit_label_undefined;
    jit_value_t lok = jit_insn_true(dsjit->fun, lhs);
    jit_insn_branch_if(dsjit->fun, lok, &ok);
    jit_value_t rok = jit_insn_true(dsjit->fun, rhs);
    jit_insn_branch_if(dsjit->fun, rok, &ok);
    jit_value_create_long_constant(dsjit->fun, jit_type_long, 0);
    jit_insn_label(dsjit->fun, &ok);
    return jit_value_create_long_constant(dsjit->fun, jit_type_long, 1);
*/
  }
//
  if(op == dsop_band)
    return jit_insn_and(dsjit->fun, lhs, rhs);
  if(op == dsop_bor)
    return jit_insn_or(dsjit->fun, lhs, rhs);
  if(op == dsop_shl)
    return jit_insn_shl(dsjit->fun, lhs, rhs);
  if(op == dsop_shr)
    return jit_insn_shr(dsjit->fun, lhs, rhs);
  exit(12);
}

static void emit_binary(DsJit *const dsjit, const DsAsStmt *stmt){
  dsjit->value[stmt->dest] = binary(dsjit, stmt);
}

static void emit_unary(DsJit *const dsjit, const DsAsStmt *stmt) {

}

static void emit_imm(DsJit *const dsjit, const DsAsStmt *stmt){
  dsjit->value[stmt->dest] = jit_value_create_long_constant(dsjit->fun, jit_type_long, stmt->num0);
}

static void emit_label(DsJit *const dsjit, const DsAsStmt *stmt) {
  jit_insn_label(dsjit->fun, &dsjit->label[stmt->dest]);
}

static void emit_if(DsJit *const dsjit, const DsAsStmt *stmt){
  if(stmt->op == dsop_imm) {
    jit_insn_branch_if(dsjit->fun, get(dsjit, stmt->num0), &dsjit->label[stmt->num1]);
    jit_insn_branch(dsjit->fun, &dsjit->label[stmt->dest]);
  } else {
    DsAsStmt tmp = {
      .op = stmt->op - dsop_if_add + dsop_add,
      .num0 = stmt->num0,
      .num1 = stmt->num1
    };
    const jit_value_t cond = binary(dsjit, &tmp);
    jit_insn_branch_if_not(dsjit->fun, cond, &dsjit->label[stmt->num1]);
  }
}

static void emit_goto(DsJit *const dsjit, const DsAsStmt *stmt){
  jit_insn_branch(dsjit->fun, &dsjit->label[stmt->dest]);
}

#define PARAM_SIZE 16
static jit_function_t launcher(DsJit *const dsjit, DsJitFun *const fun, const uint32_t i) {
  jit_function_t launcher = jit_function_create(dsjit->ctx, fun->sig);

  jit_constant_t data = { .type = jit_type_void_ptr, .un = { .ptr_value = dsjit->dsc->fun_data[i].code }};
  jit_value_t val  = jit_value_create_constant(launcher, &data);
  // optimize this?
  jit_value_t idx = jit_value_create_long_constant(launcher, jit_type_nint, 2);
  jit_value_t opt_code = jit_insn_load_elem(launcher, val, idx, jit_type_void_ptr);

  // make args
  jit_value_t arg[PARAM_SIZE];
  for(uint32_t i = 0; i < fun->narg; i++)
    arg[i] = jit_value_get_param(launcher, i);

  // check optimized code
  jit_label_t basic_call = jit_label_undefined;
  jit_constant_t null_data = { .type = jit_type_void_ptr, .un = { .ptr_value = NULL }};
  jit_value_t null  = jit_value_create_constant(launcher, &null_data);
  jit_value_t cond = jit_insn_eq(launcher, opt_code, null);
  jit_insn_branch_if(launcher, cond, &basic_call);

  // call optimized code
  jit_value_t opt_ret = jit_insn_call_indirect(launcher, opt_code, fun->sig,
            arg, fun->narg, JIT_CALL_NOTHROW);
  jit_insn_return(launcher, opt_ret);

  // call standard code
  jit_insn_label(launcher, &basic_call);
  jit_value_t std_ret = jit_insn_call(launcher, fun->name,
            fun->code, fun->sig, arg, fun->narg,
            JIT_CALL_NOTHROW);
  jit_insn_return(launcher, std_ret);

  jit_function_compile(launcher);
  return launcher;
}

static jit_value_t get_launcher(DsJit *const dsjit, DsJitFun *const fun, const uint32_t i, const uint32_t offset) {
  if(!fun->launcher) fun->launcher = launcher(dsjit, fun, i);
  return jit_insn_call(dsjit->fun, fun->name,
            fun->launcher, fun->sig, dsjit->value + offset, fun->narg,
            JIT_CALL_NOTHROW);
}

static void emit_call(DsJit *const dsjit, const DsAsStmt *stmt){
  for(uint32_t i =0; i < dsjit->nfun; i++) {
    const DsJitFun fun = dsjit->fun_data[i];
    if(stmt->name == fun.name) {
      dsjit->value[stmt->dest] = dsjit->dsc
        ? get_launcher(dsjit, &fun, i, stmt->num1)
        : jit_insn_call(dsjit->fun, stmt->name,
            fun.code, fun.sig, dsjit->value + stmt->num1, fun.narg,
            JIT_CALL_NOTHROW);
      return;
    }
  }
  exit(13);
}

static void emit_return(DsJit *const dsjit, const DsAsStmt *stmt){
  if(stmt->num0) jit_insn_default_return(dsjit->fun);
  else jit_insn_return(dsjit->fun, get(dsjit, stmt->dest));
}

typedef void (*dsjit_t)(DsJit *const, const DsAsStmt*);

static const dsjit_t functions[dsas_function] = {
  emit_binary,
  emit_unary,
  emit_imm,
  emit_label,
  emit_if,
  emit_goto,
  emit_call,
  emit_return
};

// make it match
#define PARAM_SIZE 16

ANN static void mk_trampoline(const DsJit *dsjit, DsJitFun *fun) {
  jit_type_t params[1] = { jit_type_void_ptr };
  jit_type_t sig = jit_type_create_signature
    (jit_abi_cdecl, fun->rtype, params, 1, 1);
  jit_function_t code = jit_function_create(dsjit->ctx, sig);
  jit_value_t reg = jit_value_get_param(code, 0),
              arg[PARAM_SIZE];

  for(uint32_t i = 0; i < fun->narg; i++) {
    jit_value_t idx = jit_value_create_long_constant(dsjit->fun, jit_type_nint, i);
    arg[i] = jit_insn_load_elem(code, reg, idx, jit_type_long); // how do we get sig?
  }
  jit_value_t ret = jit_insn_call(code, fun->name,
        fun->code, fun->sig, arg, fun->narg, JIT_CALL_NOTHROW);
  jit_insn_return(code, ret);
  jit_function_compile(code);
  fun->trampoline = code;
}

ANN void dsjit_compile(DsJit *const dsjit, const DsAs *dsas) {
  uint32_t start = 0;
  uint32_t narg = 0;
  uint32_t nfun = 0;
  jit_type_t params[PARAM_SIZE];
  char *name = NULL;
  for(uint32_t i = 0; i < dsas->n; i++) {
    const DsAsStmt stmt = dsas->stmts[i];
    if(stmt.type == dsas_function) {
      name = stmt.name;
      if(start)dsjit->fun = dsjit->curr->code;
// we check for new coe here
/*
if(start) {
DsJitFun *fun = dsjit->curr;
        jit_constant_t data = { .type = jit_type_void_ptr, .un = { .ptr_value = dsjit->dsc->fun_data[i].code }};
        jit_value_t val  = jit_value_create_constant(dsjit->fun, &data);
        jit_value_t idx = jit_value_create_long_constant(dsjit->fun, jit_type_nint, 2);
        jit_value_t code = jit_insn_load_elem(dsjit->fun, val, idx, jit_type_void_ptr);
        jit_value_t zero = jit_value_create_long_constant(dsjit->fun, jit_type_void_ptr, 0);
        jit_value_t cond = jit_insn_eq(dsjit->fun, code, zero);
        jit_label_t run = jit_label_undefined;
        jit_insn_branch_if(dsjit->fun, &run);

        jit_value_t arg[PARAM_SIZE];
        for(uint32_t j = 0; j < fun->narg; j++)
          arg[i] = jit_value_get_param(code, i);
      jit_value_t ret = jit_insn_call_indirect(dsjit->fun,
        fun->code, fun->sig, arg, fun->narg, JIT_CALL_NOTHROW);
jit_insn_return(dsjit->fun, ret);
       jit_insn_label(dsjit->fun , &run);
}
*/
      for(size_t j = start; j < i; j++) {
        const DsAsStmt stmt = dsas->stmts[j];
        functions[stmt.type](dsjit, &stmt);
      }
      jit_function_compile(dsjit->fun);
      dsjit->curr = &dsjit->fun_data[nfun];
      narg = 0; // should use curr->narg
      if(start) {
        if(dsjit->dsc && dsjit->curr) {
          mk_trampoline(dsjit, dsjit->curr);
          if(dsjit->nfun == dsjit->dsc->nfun - 1) break;
        }
      }
    } else if(stmt.type == dsas_arg) {
  //    gcc_jit_type *const t = get_type(dsgcc, *stmt.name);
      if(stmt.num1)
        params[narg++] = jit_type_long;
      else {
      dsjit->curr = &dsjit->fun_data[dsjit->nfun++];
      dsjit->curr->rtype = jit_type_long;
        dsjit->curr->sig = jit_type_create_signature
         (jit_abi_cdecl, jit_type_long, params, narg, 1);
        dsjit->curr->code = jit_function_create(dsjit->ctx, dsjit->curr->sig);
        jit_function_set_optimization_level(dsjit->curr->code, 1);
        dsjit->curr->narg = narg;
        dsjit->curr->name = name;
        start = i + 1;
// here we should add the check


      }
    } else if(stmt.type == dsas_label)
      dsjit->label[stmt.dest] = jit_label_undefined;
  }
  if(!dsjit->dsc) {
    if(start)dsjit->fun = dsjit->curr->code;
    for(size_t j = start; j < dsas->n; j++) {
      const DsAsStmt stmt = dsas->stmts[j];
      functions[stmt.type](dsjit, &stmt);
    }
    jit_function_compile(dsjit->fun);
  }
}

// we prob don't need dscjit curr

#ifndef DS_NOJIT
ANN int jit_launch_jit(void *data) {
  const Jitter *jitter = (Jitter*)data;
  Dsc *const dsc = jitter->dsc;
  const DsAs *dsas = jitter->dsas;
  jit_context_t ctx = jit_context_create();
  DsJit dsjit = { .ctx = ctx, .dsc = dsc};
  dsjit_compile(&dsjit, dsas);
  jit_context_build_end(ctx);
  for(size_t i = 0; i < dsc->nfun - 1; i++) {
    DscFun *fun = &dsc->fun_data[i];
    *(void**)(fun->code + sizeof(void*)) = jit_function_to_closure(dsjit.fun_data[i].trampoline);
  }
  thrd_exit(EXIT_SUCCESS);
}
#endif
