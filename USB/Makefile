obj-m	+= usbdrv1.o
obj-m   += usbdrv2.o
obj-m   += usbdrv3.o
obj-m   += usbdrv4.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
