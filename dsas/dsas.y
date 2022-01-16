%define api.pure full
%define api.prefix {dsas}
%parse-param { DsAs* arg }
%lex-param   { void *scan }

%{
#include <stdint.h>
#include "ds.h"
#include "dsas.h"

#define scan arg->scanner

DsAsStmt *stmt_last(void);
static inline void dsaserror(void *scanner, const char *str) { (void)scanner; puts(str); }
void stmt_alloc();
#define MAKE_STMT(t, ...) do { \
    arg->stmts[arg->n++]  = (DsAsStmt) { .type = dsas_##t, __VA_ARGS__ }; \
    stmt_alloc(arg);                                                \
  } while(0)
%}

%union {
  DsAsStmt stmt;
  char     *id;
  uintptr_t num;
  float     fnum;
  ds_opcode op;
};

%{
#include "dsc_lexer.h"
%}

%token DS_IMM "imm" DS_CALL "call" DS_RETURN "return" DS_IF "if" DS_GOTO "goto"
%token<op> DS_BINOP "<binop>" DS_UNOP "<unop>" DS_OP "<op>"

%token<num> DS_NUM "<integer>" DS_REG "<register>" DS_LABEL "<label>"
%token<fnum> DS_FNUM "<float>"

%token<id>  DS_ID "<id>" DS_FUN "<function>"

%type<id> argument
%type<stmt>statement

%%

functions: function | functions function

function:
  "<function>" { MAKE_STMT(function, .name = $1); }
     argument statements

statements:
  statement | statements statement

statement:
  "<binop>"  "<register>" "<register>" "<register>"
    { MAKE_STMT(binary,  .op = $1, .num0 = $2,  .num1 = $3, .dest=$4); } |
  "<op>" "<register>" "<register>" "<register>"
    { MAKE_STMT(binary,  .op = $1, .num0 = $2,  .num1 = $3, .dest=$4); } |
  "<binop>"  "<register>" "<integer>" "<register>"
    { MAKE_STMT(binary,  .op = $1 + dsop_add_imm - dsop_add, .num0 = $2,  .num1 = $3, .dest=$4); } |
  "<op>" "<register>" "<integer>" "<register>"
    { MAKE_STMT(binary,  .op = $1 + dsop_add_imm -dsop_add, .num0 = $2,  .num1 = $3, .dest=$4); } |
  "<op>"   "<register>" "<register>"
    { MAKE_STMT(unary,   .op = $1,  .num0 = $2, .dest = $3); } |
  "<unop>"   "<register>" "<register>"
    { MAKE_STMT(unary,   .op = $1,  .num0 = $2, .dest = $3); } |
  "imm"     "<integer>" "<register>"
    { MAKE_STMT(imm,     .num0 = $2,  .dest = $3); } |
  "call"    "<id>"    "<register>" "<register>"
    { MAKE_STMT(call,    .name = $2, .num1 = $3,  .dest = $4); } |
  "return"
    { MAKE_STMT(return, .num0 = 1); } |
  "return" "<register>"
    { MAKE_STMT(return, .dest = $2); } |
  "<label>"
    { MAKE_STMT(label, .dest = $1); } |
  "if" "<register>" "<label>" "<label>"
    { MAKE_STMT(if, .op = dsop_imm, .num0 = $2, .num1 = $3, .dest = $4); } |
  "if" "<op>" "<register>" "<register>" "<label>"
    { MAKE_STMT(if, .op = $2 + dsop_if_add - dsop_add, .num0 = $3, .num1 = $4, .dest = $5); } |
  "if" "<binop>" "<register>" "<register>" "<label>"
    { MAKE_STMT(if, .op = $2 + dsop_if_add - dsop_add, .num0 = $3, .num1 = $4, .dest = $5); } |
  "if" "<op>" "<register>" "<integer>" "<label>"
    { MAKE_STMT(if, .op = $2 + dsop_if_add_imm - dsop_add, .num0 = $3, .num1 = $4, .dest = $5); } |
  "if" "<binop>" "<register>" "<integer>" "<label>"
    { MAKE_STMT(if, .op = $2 + dsop_if_add_imm - dsop_add, .num0 = $3, .num1 = $4, .dest = $5); }

argument:
  argument "<id>" { stmt_last()->num1 = 1; MAKE_STMT(arg,    .name = $2); } |
  "<id>" { MAKE_STMT(arg,    .name = $1);}

%%

#define BUMP_SIZE 4096
static DsAsStmt stmt_data[BUMP_SIZE];
static unsigned int stmt_offset;

DsAsStmt *stmt_start(void) {
  return stmt_data + stmt_offset;
}

DsAsStmt *stmt_last(void) {
  return stmt_data + stmt_offset - 1;
}

void stmt_alloc() {
  stmt_offset++;
}

void stmt_release(const unsigned int n) {
  stmt_offset -= n;
}
