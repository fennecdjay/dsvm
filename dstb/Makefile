CFLAGS += -Wall -Wextra
CFLAGS += -O3
CFLAGS += -I../ds -I../dsas -I../dsc
CFLAGS +=  -I/media/DOC3/Gwion/tilde-backend/src

dstb_test: dstb_test.c libdstb.a
	${CC} ${CFLAGS} $^ -o $@ ../dsas/libdsas.a ../../tilde-backend/build/tinybackend.a

libdstb.a: dstb.o
	ar rcs $@ $^

clean:
	rm -rf dstb_test *.o *.a
