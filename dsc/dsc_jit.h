#ifndef DSC_NOJIT
#include "threads.h"

#ifdef DS_GCCJIT
ANN int gcc_launch_jit(void *data);
#define launch_jit gcc_launch_jit
#endif

#ifdef DS_JIT
ANN int jit_launch_jit(void *data);
#define launch_jit jit_launch_jit
#endif

#ifdef DS_TBJIT
ANN int tb_launch_jit(void *data);
#define launch_jit tb_launch_jit
#endif

#ifdef DS_GCCJIT
#ifdef DS_JIT
#undef launch_jit
ANN int launch_jit(void *data) {
  thrd_t thrd;
  thrd_create(&thrd, jit_launch_jit, data);
  thrd_detach(thrd);

//   enum {SECS_TO_SLEEP = 0, NSEC_TO_SLEEP = 50000000};
//   struct timespec remaining, request = {SECS_TO_SLEEP, NSEC_TO_SLEEP};
//thrd_sleep(&request, &remaining);
//  thrd_yield();
  gcc_launch_jit(data);
//  jit_launch_jit(data);
}
#endif
#endif

#endif
