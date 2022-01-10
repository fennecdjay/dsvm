#include "tb/tb.h"
#include <time.h>

#include "ds.h"
#include "dsc.h"
#include "generated/dsc_parser.h"
#include "generated/dsc_lexer.h"

TB_Function *dtb_compile(TB_Module* module, DsScanner *ds);

int main(int argc, char** argv) {
  DsScanner ds = {};
  dslex_init(&ds.scanner);
  dsset_extra(&ds, ds.scanner);

	TB_FeatureSet features = { 0 };

#if _WIN32
	TB_Module* m = tb_module_create(TB_ARCH_X86_64, TB_SYSTEM_WINDOWS, &features);
#else
	TB_Module* m = tb_module_create(TB_ARCH_X86_64, TB_SYSTEM_LINUX, &features);
#endif


{   // get the AST
    FILE *file= strcmp(argv[1], "-") ?
      fopen(argv[1], "r"):
      stdin;
    dsset_in(file, ds.scanner);
    ds.stmts = stmt_start();
    bool ret = dsparse(&ds);
    fclose(file);
    if(ret) exit(1);
}

{
  TB_Function *fib_func = dtb_compile(m, &ds);
	tb_module_compile_func(m, fib_func);
	tb_module_compile(m);
	tb_module_export_jit(m);

  typedef int(*FibFunction)(int n);
  FibFunction compiled_fib_func = (FibFunction)tb_module_get_jit_func(m, fib_func);
  int t = compiled_fib_func(2);

}

	tb_module_destroy(m);
  dslex_destroy(ds.scanner);
	return 0;
}
