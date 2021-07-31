PWD   := $(shell pwd)
obj-m += debugfs1.o
obj-m += debugfs2.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) 
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
