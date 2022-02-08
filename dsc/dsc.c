#include "ds.h"
#include "dsas.h"
#include "dsc.h"
#include "dsc_jit.h"

//typedef enum dsct_t { dsc_num, dsc_fnum } dsct_t;
//#define TYPE_SIZE 256
//static dsct_t type_data[TYPE_SIZE];

static void emit_binary(Dsc *const dsc, const DsAsStmt *stmt) {
  // handle type data when we add floats
  (void)dsc;
  dscode_binary(stmt->op, stmt->num0, stmt->num1, stmt->dest);
//  type_data[stmt->dest] = type_data[stmt->num0];
}

static ds_opcode get_unary(const ds_opcode op) {
  if(op == dsop_sub)  return dsop_neg;
  if(op == dsop_not)  return dsop_not;
  if(op == dsop_cmp)  return dsop_cmp;
  if(op == dsop_add)  return dsop_abs;
  if(op == dsop_mov)  return dsop_mov;
  if(op == dsop_mul)  return dsop_deref;
  if(op == dsop_band) return dsop_addr;
  printf("operator not found");
  exit(EXIT_FAILURE);
}

static void emit_unary(Dsc *const dsc, const DsAsStmt *stmt) {
  (void)dsc;
  const ds_opcode op = get_unary(stmt->op);
  dscode_unary(op, stmt->num0, stmt->dest);
}

static void emit_imm(Dsc *const dsc, const DsAsStmt *stmt) {
  (void)dsc;
  dscode_imm(stmt->num0, stmt->dest);
  //type_data[stmt->dest] = dsc_num;
}

static void emit_goto(Dsc *const dsc, const DsAsStmt *stmt) {
  (void)dsc;
  dscode_goto(stmt->dest);
}

static void emit_if(Dsc *const dsc, const DsAsStmt *stmt) {
  dsc->curr->if_data[dsc->curr->nif++] = (DscJump) { .stmt=*stmt, .code=dscode_start() };
  if(stmt->op == dsop_imm)
    dscode_if(stmt->num0);
  else
    dscode_if_op(stmt->op, stmt->num0, stmt->num1);
}

static void emit_call(Dsc *const dsc, const DsAsStmt *stmt) {
//  dsc->curr->call_data[dsc->curr->ncall++] =
//     (DscJump){ .stmt = *stmt, .code = dscode_start() };
//      dscode_call(NULL, stmt->num1, stmt->dest);
  for(uint32_t i =0; i < dsc->nfun; i++) {
    if(stmt->name == dsc->fun_data[i].name) {
      dscode_call(dsc->fun_data[i].code, stmt->num1, stmt->dest);
      return;
    }
  }
  printf("function %s not found\n", stmt->name);
  exit(EXIT_FAILURE);
}

static void emit_return(Dsc *const dsc, const DsAsStmt *stmt) {
  (void)dsc;
  dscode_return(stmt->dest);
}

static void emit_label(Dsc *const dsc, const DsAsStmt *stmt) {
  dsc->curr->label_data[stmt->dest] = dscode_start();
}

typedef void (*dsc_t)(Dsc *const dsc, const DsAsStmt*);

static const dsc_t functions[dsas_function] = {
  emit_binary,
  emit_unary,
  emit_imm,
  emit_label,
  emit_if,
  emit_goto,
  emit_call,
  emit_return,
};

static void scan(Dsc *const dsc, const DsAs *ds) {
  uint32_t start = 0;
  for(size_t i = 0; i < ds->n; i++) {
    const DsAsStmt stmt = ds->stmts[i];
    if(stmt.type == dsas_function) {
      if(dsc->nfun)
        dsc->fun_data[dsc->nfun-1].n = i - start;
      dsc->fun_data[dsc->nfun++].name = stmt.name;
    } else if(stmt.type == dsas_arg && !stmt.num1) {
      start = i + 1;
      dsc->fun_data[dsc->nfun-1].start = i + 1;
    }
  }
  dsc->fun_data[dsc->nfun-1].n = ds->n - start;
}

ANN static void set_labels(const Dsc *dsc, const DscFun *fun) {
  for(size_t i = 0; i < fun->nif; i++) {
    DscJump jump = fun->if_data[i];
    const DsAsStmt stmt = jump.stmt;
    dscode_t *const code = jump.code;
    if(stmt.type == dsas_if) {
      if(stmt.op == dsop_imm) {
        dscode_set_if(code, fun->label_data[stmt.num1]);
        dscode_set_else(code, fun->label_data[stmt.dest]);
      } else
        dscode_if_dest(code, fun->label_data[stmt.dest]);
    } else
      dscode_set_goto(code, fun->label_data[stmt.dest]);
  }
}
/*
ANN static void set_call(const Dsc *dsc, const DscFun *fun) {
  for(size_t i = 0; i < fun->ncall; i++) {
    DscJump jump = fun->call_data[i];
    for(uint32_t j = 0; j < dsc->nfun; j++) {
      if(jump.stmt.name == dsc->fun_data[j].name) {
        dscode_set_call(jump.code, dsc->fun_data[j].code);
        break;
      }
      printf("function %s not found\n", jump.stmt.name);
      exit(EXIT_FAILURE);
    }
  }
}
*/

ANN static void finish(const Dsc *dsc, const DscFun *fun, const dscode_t *end) {
  set_labels(dsc, fun);
//  set_call(dsc, fun);
  DsThread thread = { .code = fun->code };
  dsvm_run(&thread, end);
}

#ifndef DSC_NOJIT
thread_local static Jitter jitter;
ANN static void jit_launch(Dsc *const dsc, DsAs *const dsas) {
  jitter.dsc = dsc;
  jitter.dsas = dsas;
  thrd_t thrd;
  thrd_create(&thrd, launch_jit, &jitter);
  thrd_detach(thrd);
}
#else
#define jit_launch(a, b)
#endif

void dsc_compile(Dsc *const dsc, DsAs *const dsas) {
  scan(dsc, dsas);
  for(uint32_t i = 0; i < dsc->nfun; i++) {
    DscFun *fun = dsc->curr = &dsc->fun_data[i];
    fun->code = dscode_start();
#ifndef DSC_NOJIT
    (void)dscode_jit();
    sprintf(fun->trampoline_name, "%s____trampoline", fun->name);
#endif
    for(size_t j = 0; j < fun->n; j++) {
      const DsAsStmt stmt = dsas->stmts[fun->start + j];
      functions[stmt.type](dsc, &stmt);
    }
  }
  jit_launch(dsc, dsas);
  for(uint32_t i = 0; i < dsc->nfun - 1; i++) {
    DscFun *fun = &dsc->fun_data[i];
    finish(dsc, fun, dsc->fun_data[i+1].code);
  }
  finish(dsc, &dsc->fun_data[dsc->nfun - 1], dscode_start());
}
