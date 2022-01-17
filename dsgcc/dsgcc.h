typedef struct {
  char *name;
  gcc_jit_function *code;
  gcc_jit_function *trampoline;
  gcc_jit_type *ret_type; // jit
  uint32_t narg;
} Fun;

#define DSGCC_VALUE_SIZE 256
#define DSGCC_FUN_SIZE 256
#define DSGCC_LABEL_SIZE 64

#define LOC NULL

typedef struct DsGcc {
  gcc_jit_context *ctx;
  Fun *curr;
  Dsc *dsc;
  gcc_jit_rvalue* value_data[DSGCC_VALUE_SIZE];
  Fun fun_data[DSGCC_FUN_SIZE];
  gcc_jit_block *label_data[DSGCC_LABEL_SIZE];
  gcc_jit_block* block;
  uint32_t nfun;
  bool is_terminated;
} DsGcc;


ANN void dsgcc_compile(DsGcc *const dsgcc, const DsAs *ds);
