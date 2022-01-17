#ifndef DSC_NOJIT
#include "threads.h"

#ifdef DS_GCCJIT
ANN int gcc_launch_jit(void *data);
#define launch_jit gcc_launch_jit
#endif

#ifdef DS_TBJIT
ANN int tb_launch_jit(void *data);
#define launch_jit tb_launch_jit
#endif

#endif
