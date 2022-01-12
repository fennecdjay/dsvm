CFLAGS += -Wall -Wextra
CFLAGS += -fPIC -I. -Igenerated -I../ds -I../dsas
CFLAGS += -O2
#CFLAGS += -flto

ifeq ($(shell uname -m), x86_64)
CFLAGS += -march=native
endif

#LDFLAGS += -flto

all: dsc_test

#dsc_test: dsc_test.c libdsc.a libdsc-grammar.a
dsc_test: dsc_test.c libdsc.a ../dsas/libdsas.a
	${CC} ${CFLAGS} $^ -o $@ -I../ds ../ds/libds.a

libdsc.a: dsc.o
	ar rcs $@ $^

clean:
	rm -f *.o *.a dsc_test

.PHONY: lib
