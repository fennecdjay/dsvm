typedef struct DscJump {
  DsAsStmt  stmt;
  dscode_t *code;
} DscJump;

typedef struct DscFun {
  char  *name;
  dscode_t *code;
  char  trampoline_name[256];
  uint32_t n;
  uint32_t if_count;
  uint32_t start;
} DscFun;

#define DSC_LABEL_SIZE 256
typedef struct Dsc {
  DscFun fun_data[256];
  DscFun *curr;
  DscJump if_data[DSC_LABEL_SIZE];
  dscode_t *label_data[DSC_LABEL_SIZE];
  uint32_t fun_count;
} Dsc;
#undef DSC_LABEL_SIZE

void dsc_compile(Dsc *const dsc, const DsAs *ds);


typedef struct Jitter {
  Dsc *const dsc;
  const DsAs *dsas;
} Jitter;
