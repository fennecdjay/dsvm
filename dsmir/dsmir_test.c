#include <mir.h>
#include <mir-gen.h>
#include "ds.h"
#include "dsas.h"
#include "dsc.h"
#include "dsmir.h"

int main(int argc, char **argv) {
int thread = 0;
  if(argc < 2) exit(EXIT_FAILURE);

  DsAs dsas = {};
  dsas_init(&dsas);
  if(dsas_filename(&dsas, argv[1]))exit(EXIT_FAILURE);
  dsas_destroy(&dsas);

  MIR_context_t ctx = MIR_init();
  MIR_gen_init(ctx, thread + 1);
  MIR_gen_init(ctx, 1);
  MIR_gen_set_optimize_level(ctx, thread, 3 /*-O3*/);

  MIR_module_t m = MIR_new_module(ctx, "m");
  DsMir dsmir = { .ctx = ctx };
  dsmir_compile(&dsmir, &dsas);
  MIR_finish_module(ctx);
  MIR_load_module(ctx, m);
  MIR_link(ctx, MIR_set_gen_interface, NULL);

  void *gen = MIR_gen(ctx, thread, dsmir.fun_data[dsmir.nfun-1].fun);
  uint64_t (*fibonacci)() = (uint64_t(*)())gen;
  printf("%s(): %li\n", dsmir.fun_data[dsmir.nfun-1].name, (unsigned long)fibonacci(42));
  MIR_gen_finish(ctx);
  MIR_finish(ctx);
  return EXIT_SUCCESS;
}
