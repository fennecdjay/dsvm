CFLAGS += -Wall -Wextra
CFLAGS += -O3
CFLAGS += -I../ds -I../dsas -I.

CFLAGS += -I../dsc

all: dsjit_test libdsjit.a

dsjit_test: dsjit_test.o dsjit.c ../dsas/libdsas.a
	${CC} ${CFLAGS} -D DS_NOJIT $^ -o $@ ../dsas/libdsas.a -l jit

libdsjit.a: dsjit.o
	ar rcs $@ $^

clean:
	rm -rf dsjit_test *.o *.a
