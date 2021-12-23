all: dsc

ds:
	${MAKE} -C ds

dsc: ds
	${MAKE} -C dsc

clean:
	${MAKE} -C ds clean
	${MAKE} -C dsc clean

.PHONY: ds dsc
