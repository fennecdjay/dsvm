#include "ds.h"
#include "dsas.h"
#include "dsc.h"

int main(int argc, char **argv) {

  if (argc < 2) exit(EXIT_FAILURE);

  DsAs ds = {};
  dsas_init(&ds);
  if(dsas_filename(&ds, argv[1]))
    exit(EXIT_FAILURE);

  Dsc dsc = {};
  dsc_compile(&dsc, &ds);
  if(dsc.nfun) {
    reg_t reg[256] = {};
    DsFrame frames[256] = {};
    reg_t end_code[2] = { dsop_end };
    DsThread _thread = { .code = (dscode_t*)end_code };
    dsvm_run(&_thread, (dscode_t*)(end_code + 1));

    DsThread thread = {
      .code = dsc.fun_data[dsc.nfun-1].code,
      .reg = reg,
      .frames = frames,
      .nframe = 1
    };
    frames->reg =  reg;
    frames->code = (dscode_t*)end_code;
    dsvm_run(&thread, 0);
    printf("result: %li\n", reg[frames->out]);
  }
  dsas_destroy(&ds);
  return EXIT_SUCCESS;
}
