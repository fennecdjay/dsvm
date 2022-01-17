#include "ds.h"
#include "dsas.h"
#include "dsc.h"

#include "dsc_jit.h"

//typedef enum dsct_t { dsc_num, dsc_fnum } dsct_t;
//#define TYPE_SIZE 256
//static dsct_t type_data[TYPE_SIZE];

static void dsc_emit_binary(Dsc *const dsc, const DsAsStmt *stmt) {
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
  // if(op == dsop_band)
  return dsop_addr;
}

static void dsc_emit_unary(Dsc *const dsc, const DsAsStmt *stmt) {
  (void)dsc;
  const ds_opcode op = get_unary(stmt->op);
  dscode_unary(op, stmt->num0, stmt->dest);
}

static void dsc_emit_imm(Dsc *const dsc, const DsAsStmt *stmt) {
  (void)dsc;
  dscode_imm(stmt->num0, stmt->dest);
  //type_data[stmt->dest] = dsc_num;
}

static void dsc_emit_goto(Dsc *const dsc, const DsAsStmt *stmt) {
  (void)dsc;
  dscode_goto(stmt->dest);
}

static void dsc_emit_if(Dsc *const dsc, const DsAsStmt *stmt) {
  (void)dsc;
  if(stmt->op == dsop_imm) {
    dsc->if_data[dsc->curr->if_count++] = (DscJump) { .stmt=*stmt, .code=dscode_start() };
    dscode_if(stmt->num0);
  } else {
    dsc->if_data[dsc->curr->if_count++] = (DscJump) { .stmt=*stmt, .code=dscode_start() };
    dscode_if_op(stmt->op, stmt->num0, stmt->num1);
  }
}

static void dsc_emit_call(Dsc *const dsc, const DsAsStmt *stmt) {
  for(uint32_t i =0; i < dsc->fun_count; i++) {
    if(stmt->name == dsc->fun_data[i].name) {
      dscode_call(dsc->fun_data[i].code, stmt->num1, stmt->dest);
      return;
    }
  }
  printf("function %s not found\n", stmt->name);
  exit(EXIT_FAILURE);
}

static void dsc_emit_return(Dsc *const dsc, const DsAsStmt *stmt) {
  (void)dsc;
  dscode_return(stmt->dest);
}

static void dsc_emit_label(Dsc *const dsc, const DsAsStmt *stmt) {
  (void)dsc;
  dsc->label_data[stmt->dest] = dscode_start();
}

typedef void (*dsc_t)(Dsc *const dsc, const DsAsStmt*);

static const dsc_t functions[dsas_function] = {
  dsc_emit_binary,
  dsc_emit_unary,
  dsc_emit_imm,
  dsc_emit_label,
  dsc_emit_if,
  dsc_emit_goto,
  dsc_emit_call,
  dsc_emit_return,
};

static void scan(Dsc *const dsc, const DsAs *ds) {
  uint32_t start = 0;
  for(size_t i = 0; i < ds->n; i++) {
    const DsAsStmt stmt = ds->stmts[i];
    if(stmt.type == dsas_function) {
      if(dsc->fun_count)
        dsc->fun_data[dsc->fun_count-1].n = i - start;
      dsc->fun_data[dsc->fun_count++].name = stmt.name;
    } else if(stmt.type == dsas_arg && !stmt.num1) {
      start = i + 1;
      dsc->fun_data[dsc->fun_count-1].start = i + 1;
    }
  }
  dsc->fun_data[dsc->fun_count-1].n = ds->n - start;
}

ANN static void set_labels(const Dsc *dsc, const DscFun *fun) {
  for(size_t i = 0; i < fun->if_count; i++) {
    DscJump jump = dsc->if_data[i];
    const DsAsStmt stmt = jump.stmt;
    dscode_t *const code = jump.code;
    if(stmt.type == dsas_if) {
      if(stmt.op == dsop_imm) {
        dscode_set_if(code, dsc->label_data[stmt.num1]);
        dscode_set_else(code, dsc->label_data[stmt.dest]);
      } else
        dscode_if_dest(code, dsc->label_data[stmt.dest]);
    } else
      dscode_set_goto(code, dsc->label_data[stmt.dest]);
  }
}

void dsc_compile(Dsc *const dsc, const DsAs *dsas) {
  scan(dsc, dsas);
  for(uint32_t i = 0; i < dsc->fun_count; i++) {
    DscFun *fun = dsc->curr = &dsc->fun_data[i];
    fun->code = dscode_start();
#ifndef DSC_NOJIT
    if(i < dsc->fun_count - 1) {
      (void)dscode_jit();
      sprintf(fun->trampoline_name, "%s____trampoline", fun->name);
    }
#endif
    for(size_t j = 0; j < fun->n; j++) {
      const DsAsStmt stmt = dsas->stmts[fun->start + j];
      functions[stmt.type](dsc, &stmt);
    }
    set_labels(dsc, fun);
    DsThread thread = { .code = dsc->curr->code };
    dsvm_run(&thread, dscode_start());
  }
#ifndef DSC_NOJIT
  struct Jitter jitter = { .dsc = dsc, .dsas = dsas };
  thrd_t thrd;
  thrd_create(&thrd, launch_jit, &jitter);
  thrd_detach(thrd);
   enum {SECS_TO_SLEEP = 0, NSEC_TO_SLEEP = 50000000};
 struct timespec remaining, request = {SECS_TO_SLEEP, NSEC_TO_SLEEP};
  thrd_sleep(&request, &remaining); // sleep 1 sec);
#endif
  // set call addresses
}
