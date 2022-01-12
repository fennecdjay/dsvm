#include "ds.h"
#include "dsas.h"
#include "dsc.h"

int main(int argc, char **argv) {
  DsScanner ds = {};
  dsas_init(&ds);
  dsas_filename(&ds, argv[1]);

  Dsc dsc = {};
  dsc_compile(&dsc, &ds);
  if(dsc.fun_count) {
    reg_t reg[256];
    Frame frames[256];
    reg_t end_code[2] = { dsop_end };
    DsThread _thread = { .code = (dscode_t*)end_code };
    dsvm_run(&_thread, (dscode_t*)(end_code + 1));

    DsThread thread = {
      .code = dsc.fun_data[dsc.fun_count-1].code,
      .reg = reg,
      .frames = frames,
      .nframe = 1
    };
    frames->reg =  reg;
    frames->code = end_code;
    dsvm_run(&thread, 0);
    printf("result: %lu\n", reg[frames->out]);
  }
  dsaslex_destroy(ds.scanner);
  return EXIT_SUCCESS;
}
