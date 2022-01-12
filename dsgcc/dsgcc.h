typedef struct {
  char *name;
  gcc_jit_function *code;
  uint32_t narg;
} Fun;

#define VALUE_SIZE 256
#define FUN_SIZE 256
#define LABEL_SIZE 64

#define LOC NULL

typedef struct DsGcc {
  gcc_jit_context *ctx;
  Fun *curr;
  gcc_jit_rvalue* value_data[VALUE_SIZE];
  Fun fun_data[FUN_SIZE];
  gcc_jit_block *label_data[LABEL_SIZE];
  gcc_jit_block* block;
  uint32_t nfun;
  bool is_terminated;
} DsGcc;


ANN void dsgcc_compile(DsGcc *const dsgcc, DsAs *ds);
