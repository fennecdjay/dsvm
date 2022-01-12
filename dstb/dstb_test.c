#include "tb/tb.h"
#include "ds.h"
#include "dsas.h"
#include "dstb.h"

TB_Function *dstb_compile(TB_Module* module, DsAs *dsas);
int main(int argc, char **argv) {

  if (argc < 2) exit(EXIT_FAILURE);

  DsAs dsas = {};
  dsas_init(&dsas);
  dsas_filename(&dsas, argv[1]);
  dsas_destroy(&dsas);

  TB_FeatureSet features = {};
#if _WIN32
  TB_Module *module = tb_module_create(TB_ARCH_X86_64, TB_SYSTEM_WINDOWS, &features);
#else
  TB_Module *module = tb_module_create(TB_ARCH_X86_64, TB_SYSTEM_LINUX, &features);
#endif
  TB_Function *code = dstb_compile(module, &dsas);
  tb_module_compile(module);
//  tb_function_optimize(module, 1);
  tb_module_export_jit(module);
  typedef int64_t(*FibFunction)(void);
  FibFunction jitted_fun = (FibFunction)tb_module_get_jit_func(module, code);
  printf("result: %li\n", jitted_fun());
  tb_module_destroy(module);
  return EXIT_SUCCESS;
}
