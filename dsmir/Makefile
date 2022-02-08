CFLAGS += -Wall -Wextra
CFLAGS += -O3
CFLAGS += -I../ds -I../dsas -I.

CFLAGS += -I../dsc

dsmir_test: dsmir_test.o dsmir.c ../dsas/libdsas.a
	${CC} ${CFLAGS} -D DS_NOJIT $^ -o $@ ../dsas/libdsas.a -l mir -lpthread

libdsmir.a: dsmir.o
	ar rcs $@ $^

clean:
	rm -rf dsmir_test *.o *.a
