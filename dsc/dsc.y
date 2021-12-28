%define api.pure full
%define api.prefix {ds}
%parse-param { DsScanner* arg }
%lex-param   { void *scan }

%{
#include <stdint.h>
#include "ds.h"
#include "dsc.h"

#define scan arg->scanner

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
  ds_opcode op;
};

%{
#include "dsc_lexer.h"
%}

%token DS_IMM "imm" DS_CALL "call" DS_RETURN "return"
%token<op> DS_BINOP "<binop>" DS_IBINOP "<ibinop>" DS_JUMP "<jump>"
  DS_UNOP "<unop>"

%token<num> DS_NUM "<integer>" DS_REG "<register>"

%token<id>  DS_ID "<id>" DS_FUN "<function>"
%%

program:
  statement | program statement

statement:
  "<binop>"  "<register>" "<register>" "<register>"
    { MAKE_STMT(binary,  .op = $1, .num0 = $2,  .num1 = $3, .dest=$4); } |
  "<ibinop>" "<register>" "<register>" "<register>"
    { MAKE_STMT(binary,  .op = $1, .num0 = $2,  .num1 = $3, .dest=$4); } |
  "<ibinop>" "<register>" "<integer>" "<register>"
    { MAKE_STMT(ibinary, .op = $1, .num0 = $2,  .num1 = $3, .dest=$4); } |
  "<unop>"   "<register>" "<register>"
    { MAKE_STMT(unary,   .op = $1,  .num0 = $2, .dest = $3); } |
  "<jump>"   "<integer>"
    { MAKE_STMT(jump,    .op = $1, .dest = $2); } |
  "<jump>"    "<register>" "<register>" "<integer>"
    { MAKE_STMT(jump,    .op = $1, .num0 = $2,  .num1 = $3, .dest = $4); } |
  "imm"     "<integer>" "<register>"
    { MAKE_STMT(imm,     .num0 = $2,  .dest = $3); } |
  "<function>" { MAKE_STMT(function, .name = $1); } |
  "call"    "<id>"       "<integer>"
    { MAKE_STMT(call,    .name = $2,   .dest = $3); } |
  "return"
    { MAKE_STMT(return); }

%%
