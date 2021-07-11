PWD   := $(shell pwd)
obj-m += kmalloc.o
obj-m += vmalloc.o
obj-m += vmlist.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) 
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
