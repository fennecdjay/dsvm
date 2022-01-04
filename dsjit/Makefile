CFLAGS += -Wall -Wextra
CFLAGS += -O3 -flto

LDFLAGS += -flto

dsjit: ../dsc/libdsc.a dsjit.c
	${CC} ${CFLAGS} $^ -o $@ -I../ds -I../dsc ../dsc/libdsc.a ../ds/libds.a -l gccjit
