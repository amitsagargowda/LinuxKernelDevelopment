PWD   := $(shell pwd)
obj-m += tasklet1.o
obj-m += tasklet2.o
obj-m += tasklet3.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) 
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
