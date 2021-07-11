PWD   := $(shell pwd)
obj-m += wait1.o
obj-m += wait2.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) 
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
