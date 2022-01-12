CFLAGS += -Wall -Wextra
CFLAGS += -O3 -flto
CFLAGS += -I../ds -I../dsas -I.
LDFLAGS += -flto

dsgcc_test: dsgcc_test.o libdsgcc.a ../dsas/libdsas.a
	${CC} ${CFLAGS} $^ -o $@ ../dsas/libdsas.a -l gccjit

libdsgcc.a: dsgcc.o
	ar rcs $@ $^

clean:
	rm -rf dsgcc_test *.o *.a
