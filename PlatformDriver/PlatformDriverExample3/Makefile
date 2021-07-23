PWD   := $(shell pwd)
obj-m += plat3.o
obj-m += plat3_ins.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD)

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
