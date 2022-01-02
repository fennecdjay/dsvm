%define api.pure full
%define api.prefix {ds}
%parse-param { DsScanner* arg }
%lex-param   { void *scan }

%{
#include <stdint.h>
#include "ds.h"
#include "dsc.h"

#define scan arg->scanner

DsStmt *stmt_last(void);
static inline void dserror(void *scanner, const char *str) { (void)scanner; puts(str); }

#define MAKE_STMT(t, ...) do { \
    arg->stmts[arg->n++]  = (DsStmt) { .type = dsc_##t, __VA_ARGS__ }; \
    stmt_alloc(arg);                                                \
  } while(0)
#define DS_STMT(t, ...) do { \
    arg->stmts[arg->n++]  = (DsStmt) { __VA_ARGS__ }; \
    stmt_alloc(arg);                                                \
  } while(0)
%}

%union {
  char     *id;
  uintptr_t num;
  float     fnum;
  ds_opcode op;
};

%{
#include "dsc_lexer.h"
%}

%token DS_IMM "imm" DS_CALL "call" DS_RETURN "return"
%token<op> DS_BINOP "<binop>" DS_IBINOP "<ibinop>" DS_JUMP "<jump>"
  DS_UNOP "<unop>" DS_OP "<op>"

%token<num> DS_NUM "<integer>" DS_REG "<register>" DS_LABEL "<label>"
%token<fnum> DS_FNUM "<float>"

%token<id>  DS_ID "<id>" DS_FUN "<function>"

%type<id> argument
%%

program:
  statement | program statement

statement:
  "<binop>"  "<register>" "<register>" "<register>"
    { MAKE_STMT(binary,  .op = $1, .num0 = $2,  .num1 = $3, .dest=$4); } |
  "<ibinop>" "<register>" "<register>" "<register>"
    { MAKE_STMT(binary,  .op = $1, .num0 = $2,  .num1 = $3, .dest=$4); } |
  "<ibinop>" "<register>" "<float>" "<register>"
    { MAKE_STMT(binary,  .op = $1, .fnum0 = $2,  .num1 = $3, .dest=$4); } |
  "<ibinop>" "<register>" "<integer>" "<register>"
    { MAKE_STMT(ibinary, .op = $1, .num0 = $2,  .num1 = $3, .dest=$4); } |
  "<op>" "<register>" "<register>" "<register>"
    { MAKE_STMT(binary,  .op = $1, .num0 = $2,  .num1 = $3, .dest=$4); } |
  "<op>" "<register>" "<float>" "<register>"
    { MAKE_STMT(binary,  .op = $1, .fnum0 = $2,  .num1 = $3, .dest=$4); } |
  "<op>" "<register>" "<integer>" "<register>"
    { MAKE_STMT(ibinary, .op = $1, .num0 = $2,  .num1 = $3, .dest=$4); } |
  "<op>"   "<register>" "<register>"
    { MAKE_STMT(unary,   .op = $1,  .num0 = $2, .dest = $3); } |
  "<unop>"   "<register>" "<register>"
    { MAKE_STMT(unary,   .op = $1,  .num0 = $2, .dest = $3); } |
  "<jump>"   "<label>"
    { MAKE_STMT(jump,    .op = $1, .dest = $2); } |
  "<jump>"    "<register>" "<register>" "<label>"
    { MAKE_STMT(jump,    .op = $1, .num0 = $2,  .num1 = $3, .dest = $4); } |
  "imm"     "<integer>" "<register>"
    { MAKE_STMT(imm,     .num0 = $2,  .dest = $3); } |
  "immf"     "<float>" "<register>"
    { MAKE_STMT(immf,     .num0 = $2,  .dest = $3); } |
  "<function>" { MAKE_STMT(function, .name = $1); }
     argument |
  "call"    "<id>"    "<register>" "<register>"
    { MAKE_STMT(call,    .name = $2, .num1 = $3,  .dest = $4); } |
  "return"
    { MAKE_STMT(return, .num0 = 1); } |
  "return" "<register>"
    { MAKE_STMT(return, .dest = $2); } |
  "<label>"
    { MAKE_STMT(label, .dest = $1); }

argument:
  argument "<id>" { stmt_last()->num1 = 1; MAKE_STMT(arg,    .name = $2); } |
  "<id>" { MAKE_STMT(arg,    .name = $1);}

%%

#define BUMP_SIZE 4096
static DsStmt stmt_data[BUMP_SIZE];
static unsigned int stmt_offset;

DsStmt *stmt_start(void) {
  return stmt_data + stmt_offset;
}

DsStmt *stmt_last(void) {
  return stmt_data + stmt_offset - 1;
}

void stmt_alloc() {
  stmt_offset++;
}

void stmt_release(const unsigned int n) {
  stmt_offset -= n;
}
