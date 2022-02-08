#include <mir.h>
#include "ds.h"
#include "dsas.h"
#include "dsc.h"
#include "dsmir.h"

ANN static MIR_reg_t get_binary(DsMir *const dsmir, const DsAsStmt *stmt) {
}

ANN static void emit_binary(DsMir *const dsmir, const DsAsStmt *stmt) {
  if(stmt->op < dsop_add_imm) {
//  dsmir->value_data[stmt->dest] = MIR_new_func_reg(dsmir->ctx, dsmir->func->u.func, MIR_T_I64, c);
//    dsgcc->value_data[stmt->dest] = actual_emit_binary(dsgcc, stmt, dsgcc->value_data[stmt->num1]);
  }
}

ANN static void emit_unary(DsMir *const dsmir, const DsAsStmt *stmt) {
}

ANN static void emit_imm(DsMir *const dsmir, const DsAsStmt *stmt) {
  char c[256];
  sprintf(c, "reg%lu\n", stmt->dest);
  dsmir->value_data[stmt->dest] = MIR_new_func_reg(dsmir->ctx, dsmir->func->u.func, MIR_T_I64, c);
  MIR_append_insn(dsmir->ctx, dsmir->func,
                    MIR_new_insn(dsmir->ctx, MIR_MOV, MIR_new_reg_op(dsmir->ctx, dsmir->value_data[stmt->dest]),
                    MIR_new_int_op(dsmir->ctx, stmt->num0)));
}

ANN static void emit_label(DsMir *const dsmir, const DsAsStmt *stmt) {
  MIR_append_insn(dsmir->ctx, dsmir->func, dsmir->label_data[stmt->dest]);
}

ANN static void emit_if(DsMir *const dsmir, const DsAsStmt *stmt) {
  
}

ANN static void emit_goto(DsMir *const dsmir, const DsAsStmt *stmt) {
}

ANN static void emit_call(DsMir *const dsmir, const DsAsStmt *stmt) {
}

ANN static void emit_return(DsMir *const dsmir, const DsAsStmt *stmt) {
  MIR_append_insn(dsmir->ctx, dsmir->func,
    MIR_new_ret_insn(dsmir->ctx, 1, MIR_new_reg_op(dsmir->ctx, dsmir->value_data[stmt->dest])));
}


typedef void (*dsmir_t)(DsMir *const, const DsAsStmt*);

static const dsmir_t functions[dsas_function] = {
  emit_binary,
  emit_unary,
  emit_imm,
  emit_label,
  emit_if,
  emit_goto,
  emit_call,
  emit_return
};

static void scan(DsMir *const dsmir, const DsAs *ds) {
  uint32_t start = 0;
  for(size_t i = 0; i < ds->n; i++) {
    const DsAsStmt stmt = ds->stmts[i];
    if(stmt.type == dsas_function) {
      if(dsmir->nfun)
        dsmir->fun_data[dsmir->nfun-1].n = i - start;
      dsmir->fun_data[dsmir->nfun].name = stmt.name;
    } else if(stmt.type == dsas_arg) {
      if(stmt.num1) {
        dsmir->fun_data[dsmir->nfun].narg++;
      } else {
        start = i + 1;
        dsmir->fun_data[dsmir->nfun].start = i + 1;
// buid arg list
#define PARAM_SIZE 16
 MIR_var_t arg[PARAM_SIZE];
/*
for(uint32_t i = 0; i < dsmir->nfun; i++)
{
  char name[256];
  sprintf(name, "arg%u", i);
  arg[i] = (MIR_var_t) { .type = MIR_T_I64, .name = name };
}
*/
        dsmir->fun_data[dsmir->nfun].fun = MIR_new_func_arr(dsmir->ctx,
        dsmir->fun_data[dsmir->nfun].name,  1, (MIR_type_t[]) {MIR_T_I64},
        dsmir->fun_data[dsmir->nfun].narg, arg);
        dsmir->fun_data[dsmir->nfun].fwd =  MIR_new_forward(dsmir->ctx, dsmir->fun_data[dsmir->fun_count].name);
        char name[256];
        sprintf(name, "%s_proto_", dsmir->fun_data[dsmir->nfun].name);
        dsmir->fun_data[dsmir->nfun].proto =  MIR_new_proto_arr(dsmir->ctx, name, 1,  (MIR_type_t[]) {MIR_T_P},
        dsmir->fun_data[dsmir->nfun].narg, arg);
        dsmir->nfun++;
      }
    } else if(stmt.type == dsas_arg) {
      dsmir->label_data[stmt.dest] = MIR_new_label(dsmir->ctx);
    }
  }
  if(dsmir->nfun) {
    dsmir->fun_data[dsmir->nfun-1].n = ds->n - start;
  }
}

void dsmir_compile(DsMir *const dsmir, const DsAs *dsas) {
  scan(dsmir, dsas);
  for(uint32_t i = 0; i < dsmir->nfun; i++) {
    DsMirFun fun = dsmir->fun_data[i];
    dsmir->func = fun.fun;
    for(uint32_t j = 0; j < fun.n; j++) {
      DsAsStmt stmt = dsas->stmts[fun.start + j];
      functions[stmt.type](dsmir, &stmt);
    }
    MIR_finish_func(dsmir->ctx);
  }
}
