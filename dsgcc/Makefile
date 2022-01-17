CFLAGS += -Wall -Wextra
CFLAGS += -O3
CFLAGS += -I../ds -I../dsas -I.

CFLAGS += -I../dsc

dsgcc_test: dsgcc_test.o dsgcc.c ../dsas/libdsas.a
	${CC} ${CFLAGS} -D DS_NOJIT $^ -o $@ ../dsas/libdsas.a -l gccjit

libdsgcc.a: dsgcc.o
	ar rcs $@ $^

clean:
	rm -rf dsgcc_test *.o *.a
