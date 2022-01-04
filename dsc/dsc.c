#include <stdlib.h>
#include <stdint.h>
#include "ds.h"
#include "dsc.h"
#include "dsc_parser.h"
#include "dsc_lexer.h"

typedef struct {
  char  *name;
  reg_t *code;
  uint32_t nlabel;
} Fun;

#define FUN_SIZE 4096
static Fun fun_data[FUN_SIZE];
static uint32_t fun_count;

typedef struct {
  reg_t *code;
  uint32_t n;
} Label;

#define LABEL_SIZE 4096
static Label label_data[LABEL_SIZE] = {};
static uint32_t goto_count;
static Label goto_data[LABEL_SIZE] = {};

typedef enum dsct_t { dsc_num, dsc_fnum } dsct_t;
#define TYPE_SIZE 4096
static dsct_t type_data[TYPE_SIZE];

static void dsc_emit_binary(const DsStmt *stmt) {
  const ds_opcode op = type_data[stmt->num0] == dsc_num ?
    stmt->op : stmt->op + dsop_addf - dsop_add;
  dscode_binary(op, stmt->num0, stmt->num1, stmt->dest);
  dscode_binary(stmt->op, stmt->num0, stmt->num1, stmt->dest);
  type_data[stmt->dest] = type_data[stmt->num0];
}

static void dsc_emit_ibinary(const DsStmt *stmt) {
  dscode_ibinary(stmt->op, stmt->num0, stmt->num1, stmt->dest);
  type_data[stmt->dest] = dsc_num;
}

static void dsc_emit_fbinary(const DsStmt *stmt) {
  dscode_ibinary(stmt->op + dsop_addf - dsop_add, stmt->num0, stmt->fnum1, stmt->dest);
  type_data[stmt->dest] = dsc_fnum;
}

static void dsc_emit_unary(const DsStmt *stmt) {
  if(stmt->op == dsop_add)exit(77); // TODO: abs
  const ds_opcode op = stmt->op != dsop_sub ?
    stmt->op : dsop_neg;
  dscode_unary(op, stmt->num0, stmt->dest);
  type_data[stmt->dest] = type_data[stmt->num0];
}

static void dsc_emit_imm(const DsStmt *stmt) {
  dscode_imm(stmt->num0, stmt->dest);
  type_data[stmt->dest] = dsc_num;
}

static void dsc_emit_immf(const DsStmt *stmt) {
  dscode_immf(stmt->fnum0, stmt->dest);
  type_data[stmt->dest] = dsc_fnum;
}

static void dsc_emit_jump(const DsStmt *stmt) {
  Label label = label_data[stmt->dest];
  reg_t *addr = dscode_start() + (sizeof(DsInfo3) / sizeof(reg_t)); // arch (was 2)
  if(!label.code)
    goto_data[goto_count++] = (Label) { addr, stmt->dest };
  dscode_jump_op(stmt->op, stmt->num0, stmt->num1, label.code);
}

static void dsc_emit_call(const DsStmt *stmt) {
  for(uint32_t i =0; i < fun_count; i++) {
    if(stmt->name == fun_data[i].name) {
      dscode_call(fun_data[i].code, stmt->num1, stmt->dest);
      return;
    }
  }
  exit(13);
}


static void dsc_emit_return(const DsStmt *stmt) {
  (void)stmt;
  dscode_return(stmt->dest);
//  dscode_return();
}

static inline void finish(const reg_t *code) {
  if(fun_count) {
    for(uint32_t i = 0; i < goto_count; i++)
      *(reg_t**)goto_data[i].code = label_data[goto_data[i].n].code;
    reg_t *const former = fun_data[fun_count-1].code;
    dsvm_run(former, code);
  }
  goto_count = 0;
}

static void dsc_emit_function(const DsStmt *stmt) {
  reg_t *const code = dscode_start();
  finish(code);
  fun_data[fun_count++] = (Fun) { .name = stmt->name, .code = code };
}

static void dsc_emit_argument(const DsStmt *stmt) {
  if(stmt->num1)
    type_data[stmt->dest] = *stmt->name == 'i' ? dsc_num : dsc_fnum;
// ignore ret value? hum
}

static void dsc_emit_label(const DsStmt *stmt) {
  label_data[stmt->dest].code = dscode_start();
}

typedef void (*dsc_t)(const DsStmt*);

static const dsc_t functions[dsop_max] = {
  dsc_emit_binary,
  dsc_emit_ibinary,
  dsc_emit_fbinary,
  dsc_emit_unary,
  dsc_emit_imm,
  dsc_emit_immf,
  dsc_emit_jump,
  dsc_emit_call,
  dsc_emit_return,
  dsc_emit_function,
  dsc_emit_argument,
  dsc_emit_label
};

#define BUMP_SIZE 4096
static char _data[BUMP_SIZE];

char* string_alloc(const char *c) {
  char *data = _data;
  do {
    if(*data == '\0') break;
    if(!strcmp(c, data))
      return data;
  } while((data = strchr(data, '\0') + 1));
  if(data - _data >= BUMP_SIZE)exit(3);
  strcpy(data, c);
  return data;
}

static void set_funcs_and_labels(DsScanner *ds) {
  uint32_t fun_count = 0;
  uint32_t nlabel    = 0;
  for(size_t i = 0; i < ds->n; i++) {
    const DsStmt stmt = ds->stmts[i];
    if(stmt.type == dsc_function) {
      if(fun_count)
        fun_data[fun_count - 1].nlabel = nlabel;
      fun_data[fun_count].name = stmt.name;
      nlabel = 0;
      fun_count++;
    } else if(stmt.type == dsc_label) {
      nlabel++;
    }
  }
}

int main(int argc, char **argv) {
  DsScanner ds = {};
  dslex_init(&ds.scanner);
  dsset_extra(&ds, ds.scanner);
//  for (int i = 1; i < argc; i++) {
    memset(label_data, 0, sizeof(label_data));
//    FILE *file= strcmp(argv[i], "-") ?
//      fopen(argv[i], "r"):
    FILE *file= argv[1] ?
      fopen(argv[1], "r"):
      stdin;
    if(!file) return EXIT_FAILURE; //continue;
    dsset_in(file, ds.scanner);
    ds.stmts = stmt_start();
    dsparse(&ds);
    fclose(file);
    set_funcs_and_labels(&ds);
    for(size_t i = 0; i < ds.n; i++) {
      const DsStmt stmt = ds.stmts[i];
      functions[stmt.type](&stmt);
    }
//    stmt_release(ds.n);
    dscode_end();
    finish(dscode_start());
    if(fun_count)
      dsvm_run(fun_data[fun_count-1].code, 0);
//    ds.n = 0;
//  }
  dslex_destroy(ds.scanner);
  return EXIT_SUCCESS;
}
