CFLAGS += -Wall -Wextra
CFLAGS += -O3 -flto
CFLAGS += -I../ds -I../dsas
CFLAGS +=  -I/media/DOC3/Gwion/tilde-backend/src

LDFLAGS += -flto

dstb_test: dstb_test.c libdstb.a
	${CC} ${CFLAGS} $^ -o $@ ../dsas/libdsas.a ../../tilde-backend/build/tinybackend.a

libdstb.a: dstb.o
	ar rcs $@ $^

clean:
	rm -rf dstb_test *.o *.a
