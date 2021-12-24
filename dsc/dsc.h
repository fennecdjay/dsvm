#include <stdio.h>

typedef enum {
  dsc_binary,
  dsc_ibinary,
  dsc_unary,
  dsc_imm,
  dsc_jump,
  dsc_call,
  dsc_return,
  dsc_function,
  dsc_stmt_max
} dsc_stmt_type;

typedef struct {
  union {
    char  *name;
    reg_t num0;
  };
  reg_t num1;
  reg_t dest;
  dsc_stmt_type type;
  ds_opcode op;
} DsStmt;

typedef struct DsScanner {
  void *scanner;
  DsStmt *stmts;
  uint32_t n;
  uint32_t cap;
} DsScanner;

char *string_alloc(const char*);
void stmt_alloc();
DsStmt *stmt_start(void);
#define YYSTYPE DSSTYPE
