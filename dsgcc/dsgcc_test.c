#include <libgccjit.h>
#include "ds.h"
#include "dsas.h"
#include "dsgcc.h"

int main(int argc, char **argv) {

  if(argc < 2) exit(EXIT_FAILURE);

  DsAs dsas = {};
  dsas_init(&dsas);
  if(dsas_filename(&dsas, argv[1]))exit(EXIT_FAILURE);
  dsas_destroy(&dsas);

  gcc_jit_context *ctx = gcc_jit_context_acquire();
  gcc_jit_context_set_int_option (ctx,
        GCC_JIT_INT_OPTION_OPTIMIZATION_LEVEL,
        3);

  DsGcc dsgcc = { .ctx = ctx, };
  dsgcc_compile(&dsgcc, &dsas);
  const char *name = dsgcc.curr->name;
  if(dsgcc.nfun) {
    gcc_jit_result *result = gcc_jit_context_compile (ctx);
    long(*_main)(void)  = gcc_jit_result_get_code (result, name);
    if (!_main) {
      fprintf (stderr, "NULL greet");
      exit (1);
    }
    printf("result :%lu\n", _main ());
  }
  return EXIT_SUCCESS;
}
