CFLAGS += -Wall -Wextra
CFLAGS += -fPIC -I. -Igenerated -I../ds -march=native
CFLAGS += -O3 -flto

LDFLAGS += -flto

all: lib dsc

dsc: libdsc.a dsc.c
	${CC} ${CFLAGS} $^ -o $@ -I../ds libdsc.a ../ds/libds.a

lib: parser lexer libdsc.a

libdsc.a: generated/dsc_parser.o generated/dsc_lexer.o
	ar rcs $@ $^

lexer: dsc.l
	flex --header-file=generated/dsc_lexer.h -o generated/dsc_lexer.c $<

parser: dsc.y
	bison -Wno-yacc --defines=generated/dsc_parser.h -o generated/dsc_parser.c $<

clean:
	rm -f *.o *.a dsc

.PHONY: lib
