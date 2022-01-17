CFLAGS += -Wall -Wextra
CFLAGS += -fPIC -I. -Igenerated -I../ds -I../dsas
CFLAGS += -O2 -ffast-math
CFLAGS += -std=c1x


JIT_CFLAGS += -lpthread
GCCJIT_CFLAGS += -D DS_GCCJIT -lgccjit
TBJIT_CFLAGS += -D DS_TBJIT
TBJIT_LDFLAGS += ../../tilde-backend/build/tinybackend.a

LDFLAGS += ../ds/libds.a

ifeq ($(shell uname -m), x86_64)
CFLAGS += -march=native
endif

all: dsc_nojit dsc_gcc dsc_tb

dsc_nojit: dsc_test.c dsc.c ../dsas/libdsas.a
	${CC} -D DSC_NOJIT ${CFLAGS} $^ ${LD_FLAGS} -o $@ ../ds/libds.a

dsc_gcc: dsc_test.c dsc.c ../dsas/libdsas.a ../ds/libds.a ../dsgcc/libdsgcc.a
	${CC} ${CFLAGS} ${GCCJIT_CFLAGS} ${JIT_CFLAGS} $^ -o $@ -I../ds ../ds/libds.a ../dsgcc/libdsgcc.a

dsc_tb: dsc_test.c dsc.c ../dsas/libdsas.a ../ds/libds.a ../dstb/libdstb.a
	${CC} ${CFLAGS} ${TBJIT_CFLAGS} ${JIT_CFLAGS} $^ -o $@ -I../ds ../ds/libds.a ${TBJIT_LDFLAGS}

libdsc.a: dsc.o
	ar rcs $@ $^

clean:
	rm -f *.o *.a dsc_nojit dsc_gcc

../ds/libds.a:
	${MAKE} -C ../ds libds.a

../dsas/libdsas.a:
	${MAKE} -C ../dsas libdsas.a

../dsgcc/libdsgcc.a:
	${MAKE} -C ../dsgcc libdsgcc.a

../dstb/libdstb.a:
	${MAKE} -C ../dstb libdstb.a

.PHONY: lib
