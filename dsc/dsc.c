#include "ds.h"
#include "dsas.h"
#include "dsc.h"

#define LABEL_SIZE 256

typedef struct DscJump {
  DscStmt  stmt;
  dscode_t *code;
} DscJump;

DscJump if_data[LABEL_SIZE];

static dscode_t *label_data[LABEL_SIZE];

//typedef enum dsct_t { dsc_num, dsc_fnum } dsct_t;
//#define TYPE_SIZE 256
//static dsct_t type_data[TYPE_SIZE];

static void dsc_emit_binary(Dsc *const dsc, const DscStmt *stmt) {
  (void)dsc;
//  const ds_opcode op = type_data[stmt->num0] == dsc_num ?
//    stmt->op : stmt->op + dsop_addf - dsop_add;
//  dscode_binary(op, stmt->num0, stmt->num1, stmt->dest);
  dscode_binary(stmt->op, stmt->num0, stmt->num1, stmt->dest);
//  type_data[stmt->dest] = type_data[stmt->num0];
}

static void dsc_emit_unary(Dsc *const dsc, const DscStmt *stmt) {
  (void)dsc;
  (void)stmt;
//  if(stmt->op == dsop_add)exit(77); // TODO: abs
//  const ds_opcode op = stmt->op != dsop_sub ?
//    stmt->op : dsop_neg;
//  dscode_unary(op, stmt->num0, stmt->dest);
  //type_data[stmt->dest] = type_data[stmt->num0];
}

static void dsc_emit_imm(Dsc *const dsc, const DscStmt *stmt) {
  (void)dsc;
  dscode_imm(stmt->num0, stmt->dest);
  //type_data[stmt->dest] = dsc_num;
}
/*
static void dsc_emit_immf(const DscStmt *stmt) {
  (void)dsc;
  dscode_immf(stmt->fnum0, stmt->dest);
  //type_data[stmt->dest] = dsc_fnum;
}
*/
static void dsc_emit_if(Dsc *const dsc, const DscStmt *stmt) {
  (void)dsc;
  if_data[dsc->curr->if_count++] = (DscJump) { .stmt=*stmt, .code=dscode_start() };
  dscode_if(stmt->num0);
}

static void dsc_emit_goto(Dsc *const dsc, const DscStmt *stmt) {
  (void)dsc;
  (void)stmt;
  exit(12);
}

static void dsc_emit_call(Dsc *const dsc, const DscStmt *stmt) {
  for(uint32_t i =0; i < dsc->fun_count; i++) {
    if(stmt->name == dsc->fun_data[i].name) {
      dscode_call(dsc->fun_data[i].code, stmt->num1, stmt->dest);
      return;
    }
  }
  exit(13);
}

static void dsc_emit_return(Dsc *const dsc, const DscStmt *stmt) {
  (void)dsc;
  dscode_return(stmt->dest);
}

static void dsc_emit_label(Dsc *const dsc, const DscStmt *stmt) {
  (void)dsc;
  label_data[stmt->dest] = dscode_start();
}

typedef void (*dsc_t)(Dsc *const dsc, const DscStmt*);

static const dsc_t functions[dsc_function] = {
  dsc_emit_binary,
  dsc_emit_unary,
  dsc_emit_imm,
//  dsc_emit_immf,
  dsc_emit_label,
  dsc_emit_if,
  dsc_emit_goto,
  dsc_emit_call,
  dsc_emit_return,
};

static void scan(Dsc *const dsc, const DsScanner *ds) {
  uint32_t start = 0;
  for(size_t i = 0; i < ds->n; i++) {
    const DscStmt stmt = ds->stmts[i];
    if(stmt.type == dsc_function) {
      if(dsc->fun_count)
        dsc->fun_data[dsc->fun_count-1].n = i - start;
      dsc->fun_data[dsc->fun_count++].name = stmt.name;
    } else if(stmt.type == dsc_arg && !stmt.num1) {
      start = i + 1;
      dsc->fun_data[dsc->fun_count-1].start = i + 1;
    }
  }
  dsc->fun_data[dsc->fun_count-1].n = ds->n - start;
}

ANN static void set_labels(const DscFun *fun) {
  for(size_t i = 0; i < fun->if_count; i++) {
    DscJump jump = if_data[i];
    dscode_t *const if_code = jump.code;
    dscode_set_if(if_code, label_data[jump.stmt.num1]);
    dscode_set_else(if_code, label_data[jump.stmt.dest]);
  }
}

void dsc_compile(Dsc *const dsc, const DsScanner *ds) {
  scan(dsc, ds);
  for(uint32_t i = 0; i < dsc->fun_count; i++) {
    DscFun *fun = dsc->curr = &dsc->fun_data[i];
    fun->code = dscode_start();
    for(size_t j = 0; j < fun->n; j++) {
      const DscStmt stmt = ds->stmts[fun->start + j];
      functions[stmt.type](dsc, &stmt);
    }
    set_labels(fun);
    DsThread thread = { .code = dsc->curr->code };
    dsvm_run(&thread, dscode_start());
  }
  // set call addresses
}
