#include <stdio.h>

typedef enum {
  dsas_binary,
  dsas_unary,
  dsas_imm,
//  dsas_immf,
  dsas_label,
  dsas_if,
  dsas_goto,
  dsas_call,
  dsas_return,
  dsas_function,
  dsas_arg,
  dsas_max
} dsas_type;

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
  dsas_type type;
  ds_opcode op;
} DsAsStmt;

typedef struct DsAs {
  void *scanner;
  DsAsStmt *stmts;
  uint32_t n;
  uint32_t cap;
} DsAs;


void dsas_init(DsAs *dsas);
bool dsas_compile(DsAs *dsas);
bool dsas_file(DsAs *dsas, FILE *file);
bool dsas_filename(DsAs *dsas, char* path);

void dsas_destroy(DsAs *const dsas);
DsAsStmt *stmt_start(void);
//void stmt_release(const unsigned int n);
#define YYSTYPE DSASSTYPE
