CFLAGS += -Wall -Wextra
#CFLAGS += -falign-labels -falign-jumps
CFLAGS += -fstrict-aliasing
CFLAGS += -O3 -ffast-math -fomit-frame-pointer


#LDFLAGS += -msse -msse2

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
