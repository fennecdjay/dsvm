typedef struct DscFun {
  char  *name;
  dscode_t *code;
  uint32_t n;
  uint32_t if_count;
  uint32_t start;
} DscFun;

typedef struct Dsc {
  DscFun fun_data[256];
  DscFun *curr;
  uint32_t fun_count;
} Dsc;

void dsc_compile(Dsc *const dsc, const DsAs *ds);
