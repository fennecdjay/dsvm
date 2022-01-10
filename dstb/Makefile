CFLAGS += -Wall -Wextra
#CFLAGS += -O3 -flto
CFLAGS += -O2
CFLAGS +=  -I/media/DOC3/Gwion/tilde-backend/src
#LDFLAGS += -flto

dstb: dstb.c dstb_pass.c
	${CC} ${CFLAGS} $^ -o $@ -I../ds -I../dsc ../dsc/libdsc.a ../ds/libds.a tinybackend.a
#	${CC} ${CFLAGS} $^ -o $@ -I../ds -I../dsc ../dsc/libdsc.a /media/DOC3/Gwion/tilde-backend/build/tinybackend.a
