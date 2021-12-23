#include <stdlib.h>
#include <stdint.h>
#include "ds.h"
#include "dsc.h"
#include "dsc_parser.h"
#include "dsc_lexer.h"

typedef struct {
  char  *name;
  reg_t *code;
} Fun;

#define FUN_SIZE 4096
static Fun fun_data[FUN_SIZE];
static uint32_t fun_count;

static void fun_alloc(char *name) {
  fun_data[fun_count++] = (Fun) { .name = name, .code = dscode_start() };
}

static void dsc_emit_binary(const DsStmt *stmt) {
  dscode_binary(stmt->op, stmt->num0, stmt->num1, stmt->dest);
}
static void dsc_emit_ibinary(const DsStmt *stmt) {
  dscode_ibinary(stmt->op, stmt->num0, stmt->num1, stmt->dest);
}
static void dsc_emit_unary(const DsStmt *stmt) {
  dscode_unary(stmt->op, stmt->num0, stmt->dest);
}
static void dsc_emit_imm(const DsStmt *stmt) {
  dscode_imm(stmt->num0, stmt->dest);
}
static void dsc_emit_jump(const DsStmt *stmt) {
  if(stmt->op == dsop_max)
    dscode_jump(stmt->dest);
  else
    dscode_jump_op(stmt->op, stmt->num0, stmt->num1, stmt->dest);
}

static void dsc_emit_call(const DsStmt *stmt) {
  for(uint32_t i =0; i < fun_count; i++) {
    if(stmt->id0 == fun_data[i].name) {
      dscode_call(fun_data[i].code, stmt->dest);
      return;
    }
  }
  exit(3);
}

static void dsc_emit_return(const DsStmt *stmt) {
  (void)stmt;
  dscode_return();
}
static void dsc_emit_function(const DsStmt *stmt) {
  fun_alloc(stmt->id0);
}

typedef void (*dsc_t)(const DsStmt*);

static const dsc_t functions[dsop_max] = {
  dsc_emit_binary,
  dsc_emit_ibinary,
  dsc_emit_unary,
  dsc_emit_imm,
  dsc_emit_jump,
  dsc_emit_call,
  dsc_emit_return,
  dsc_emit_function,
};

#define BUMP_SIZE 4096
static char _data[BUMP_SIZE];

char* bump_alloc(const char *c) {
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

static DsStmt stmt_data[BUMP_SIZE];
static unsigned int stmt_offset;

DsStmt *stmt_start(void) {
  return stmt_data + stmt_offset;
}

void stmt_alloc() {
  stmt_offset++;
}

void stmt_release(const unsigned int n) {
  stmt_offset -= n;
}

int main(int argc, char **argv) {
  DsScanner ds = {};
  dslex_init(&ds.scanner);
  dsset_extra(&ds, ds.scanner);
  for (int i = 1; i < argc; i++) {
    ds.file= strcmp(argv[i], "-") ?
      fopen(argv[i], "r"):
      stdin;
    if(!ds.file) continue;
    dsset_in(ds.file, ds.scanner);
    ds.stmts = stmt_start();
    dsparse(&ds);
    fclose(ds.file);
    for(size_t i = 0; i < ds.n; i++) {
      const DsStmt stmt = ds.stmts[i];
      functions[stmt.type](&stmt);
    }
    dscode_end();
    stmt_release(ds.n);
    ds.n = 0;
  }
  dslex_destroy(ds.scanner);
  dsvm_run(fun_data[fun_count-1].code);
  return EXIT_SUCCESS;
}
