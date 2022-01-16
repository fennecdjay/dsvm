CFLAGS += -Wall -Wextra
#CFLAGS += -Os -flto=full -fomit-frame-pointer -ffast-math
CFLAGS += -falign-labels -fstrict-aliasing -falign-jumps
CFLAGS += -O2 -ffast-math -fomit-frame-pointer
#CFLAGS += -flto
#LDFLAGS += -flto

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
