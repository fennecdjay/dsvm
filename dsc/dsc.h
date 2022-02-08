typedef struct DscJump {
  DsAsStmt  stmt;
  dscode_t *code;
} DscJump;

#define DSC_LABEL_SIZE 128
typedef struct DscFun {
  char  *name;
  dscode_t *code;
  char  trampoline_name[256];
  DscJump if_data[DSC_LABEL_SIZE];
  dscode_t *label_data[DSC_LABEL_SIZE];
//  DscJump   call_data[DSC_LABEL_SIZE];
  uint32_t n;
  uint32_t nif;
  uint32_t ncall;
  uint32_t start;
} DscFun;

typedef struct Dsc {
  DscFun fun_data[256];
  DscFun *curr;
  uint32_t nfun;
//  uint32_t cap
} Dsc;
#undef DSC_LABEL_SIZE

void dsc_compile(Dsc *const dsc, DsAs *const ds);

typedef struct Jitter {
  Dsc *dsc;
  DsAs *dsas;
} Jitter;
