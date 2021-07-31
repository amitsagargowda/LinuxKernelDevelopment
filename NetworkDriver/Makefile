PWD   := $(shell pwd)
obj-m += eth-ins.o
obj-m += fake-eth.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) 
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
