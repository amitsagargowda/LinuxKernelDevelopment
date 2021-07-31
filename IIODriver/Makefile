PWD   := $(shell pwd)
obj-m += iio1.o
obj-m += iio1_ins.o

all:
#	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) 
	make -C /home/test/qemu/linux-5.1.16/  ARCH=arm  CROSS_COMPILE=/home/test/qemu/gcc-linaro-10.2.1-2021.02-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf- M=$(PWD) 
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	make -C /home/test/qemu/linux-5.1.16/  ARCH=arm  CROSS_COMPILE=/home/test/qemu/gcc-linaro-10.2.1-2021.02-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf- M=$(PWD) clean
