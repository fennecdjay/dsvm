typedef struct DsJitFun {
  char *name;
  jit_function_t code;
  jit_function_t trampoline;
  jit_function_t launcher;
  jit_type_t sig;
  jit_type_t rtype;
  uint32_t narg;
} DsJitFun;

typedef struct DsJit {
  jit_function_t fun;
  jit_context_t ctx;
  DsJitFun *curr;
  Dsc *dsc;
  jit_value_t value[256];
  jit_label_t label[256];
  DsJitFun fun_data[256];
  uint32_t nfun;
} DsJit;

ANN void dsjit_compile(DsJit *const dsjit, const DsAs *dsas);
