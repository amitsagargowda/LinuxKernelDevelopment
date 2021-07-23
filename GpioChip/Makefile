PWD   := $(shell pwd)
obj-m += fake-gpio-chip.o
obj-m += fake-gpio-ins.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) 
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
