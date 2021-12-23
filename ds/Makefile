CFLAGS += -Wall -Wextra
#CFLAGS += -O2 -flto
CFLAGS += -O3

#LDFLAGS += -flto

all: main

libds.a: ds.c ds.h
	${CC} ${CFLAGS} ${LDFLAGS} -c ds.c
	ar rcs libds.a ds.o

main: libds.a
	${CC} -flto -O2 ds_test.c libds.a -o ds_test

clean:
	rm -f *.o *.a ds_test

test:
	@sh test.sh
