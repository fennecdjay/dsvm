#include <libgccjit.h>
#include "ds.h"
#include "dsas.h"
#include "dsgcc.h"

int main(int argc, char **argv) {

  gcc_jit_context *ctx = gcc_jit_context_acquire();
  gcc_jit_context_set_int_option (ctx,
        GCC_JIT_INT_OPTION_OPTIMIZATION_LEVEL,
        3);

  DsScanner ds = {};
  dsas_init(&ds);
  if(dsas_filename(&ds, argv[1]))exit(EXIT_FAILURE);
  dsaslex_destroy(ds.scanner);
  DsJit dsjit = { .ctx = ctx, };
  dsgcc_compile(&dsjit, &ds);
  if(dsjit.nfun) {
    gcc_jit_result *result = gcc_jit_context_compile (ctx);
    long(*_main)(void)  = gcc_jit_result_get_code (result, dsjit.curr->name);
    if (!_main) {
      fprintf (stderr, "NULL greet");
      exit (1);
    }
    printf("result :%lu\n", _main ());
  }
  return EXIT_SUCCESS;
}
