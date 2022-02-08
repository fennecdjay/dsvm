#include <jit/jit.h>
#include "ds.h"
#include "dsas.h"
#include "dsc.h"
#include "dsjit.h"

int main(int argc, char **argv) {

  if(argc < 2) exit(EXIT_FAILURE);

  DsAs dsas = {};
  dsas_init(&dsas);
  if(dsas_filename(&dsas, argv[1]))exit(EXIT_FAILURE);
  dsas_destroy(&dsas);

  jit_context_t ctx = jit_context_create();
//  gcc_jit_context_set_int_option (ctx,
//    GCC_JIT_INT_OPTION_OPTIMIZATION_LEVEL, 3);
  DsJit dsjit = { .ctx = ctx, };
  dsjit_compile(&dsjit, &dsas);
  jit_context_build_end(ctx);
  if(dsjit.nfun) {
    jit_long result;
    void *args[1];
    jit_function_apply(dsjit.curr->code, args, &result);
    printf("%s() = %d\n", dsjit.curr->name, (int)result);
  }
  jit_context_destroy(ctx);
  return EXIT_SUCCESS;
}
