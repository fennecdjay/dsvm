typedef struct DsMirFun {
  char *name;
  MIR_item_t fun;
  MIR_item_t fwd;
  MIR_item_t proto;
  uint32_t narg;
  uint32_t n;
  uint32_t start;
} DsMirFun;

typedef struct DsMir {
  MIR_context_t ctx;
  MIR_item_t func;
  DsMirFun fun_data[256];
  MIR_reg_t value_data[256];
  MIR_label_t label_data[256];
  uint32_t nfun;
} DsMir;

void dsmir_compile(DsMir *const dsmir, const DsAs *dsas);
