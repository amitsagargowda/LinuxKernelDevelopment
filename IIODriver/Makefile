PWD   := $(shell pwd)
obj-m += iio1.o
obj-m += iio1_ins.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) 
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
