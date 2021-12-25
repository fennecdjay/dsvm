CFLAGS += -Wall -Wextra
CFLAGS += -O2
CFLAGS += -falign-labels
#CFLAGS += -flto

ifeq ($(shell uname -m), x86_64)
CFLAGS += -march=native
endif

all: main

libds.a: ds.c ds.h
	${CC} ${CFLAGS} ${LDFLAGS} -c ds.c
	ar rcs libds.a ds.o

main: libds.a
	${CC} ${CFLAGS} ds_test.c libds.a -o ds_test

clean:
	rm -f *.o *.a ds_test

test:
	@sh test.sh
