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
    char     *id0;
    uintptr_t num0;
  };
  uintptr_t num1;
  uintptr_t dest;
  dsc_stmt_type type;
  ds_opcode op;
} DsStmt;

typedef struct DsScanner {
  void *scanner;
  DsStmt *stmts;
  FILE *file;
  unsigned int n;
  unsigned int cap;
} DsScanner;

char* bump_alloc(const char*);
void stmt_alloc();
DsStmt *stmt_start(void);
#define YYSTYPE DSSTYPE
