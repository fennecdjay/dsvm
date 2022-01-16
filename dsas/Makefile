CFLAGS += -Wall -Wextra
CFLAGS += -fPIC -I. -Igenerated -I../ds
CFLAGS += -O2 -fomit-frame-pointer -ffast-math
#CFLAGS += -flto

ifeq ($(shell uname -m), x86_64)
CFLAGS += -march=native
endif

#LDFLAGS += -flto

libdsas.a: generated/dsc_parser.o generated/dsc_lexer.o
	ar rcs $@ $^

generated/dsc_lexer.c: dsas.l dsas.h
	flex --header-file=generated/dsc_lexer.h -o generated/dsc_lexer.c $<

generated/dsc_parser.c: dsas.y dsas.h
	bison -Wno-yacc --defines=generated/dsc_parser.h -o generated/dsc_parser.c $<

clean:
	rm -f *.a
