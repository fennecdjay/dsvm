all: ds dsc dstb dsgcc

ds:
	${MAKE} -C ds

dsas: ds
	${MAKE} -C dsas

dsc: dsas
	${MAKE} -C dsc

dstb: dsas
	${MAKE} -C dstb

dsgcc: dsas
	${MAKE} -C dsgcc

dsjit: dsas
	${MAKE} -C dsjit

clean:
	${MAKE} -C ds    clean
	${MAKE} -C dsc   clean
	${MAKE} -C dstb  clean
	${MAKE} -C dsgcc clean

.PHONY: ds dsas dsc dsjit dstb
