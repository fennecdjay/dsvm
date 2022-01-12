#include <stdio.h>

typedef enum {
  dsc_binary,
  dsc_unary,
  dsc_imm,
//  dsc_immf,
  dsc_label,
  dsc_if,
  dsc_goto,
  dsc_call,
  dsc_return,
  dsc_function,
  dsc_arg,
  dsc_stmt_max
} dsc_stmt_type;

typedef struct {
  union {
    char  *name;
    reg_t num0;
    float fnum0;
  };
  union {
    reg_t num1;
    float fnum1;
  };
  reg_t dest;
  dsc_stmt_type type;
  ds_opcode op;
} DscStmt;

typedef struct DsScanner {
  void *scanner;
  DscStmt *stmts;
  uint32_t n;
  uint32_t cap;
} DsScanner;


void dsas_init(DsScanner *dsas);
bool dsas_compile(DsScanner *dsas);
bool dsas_file(DsScanner *dsas, FILE *file);
bool dsas_filename(DsScanner *dsas, char* path);

DscStmt *stmt_start(void);
//void stmt_release(const unsigned int n);
#define YYSTYPE DSASSTYPE
