PWD   := $(shell pwd)
obj-m += procfs1.o
obj-m += procfs2.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) 
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
