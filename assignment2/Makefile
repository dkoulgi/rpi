ARCH=arm

KDIR=/lib/modules/$(shell uname -r)/build

obj-m = cycle_count_mod.o

ccflags-m += -Wall

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules
	gcc assignment2.c -lgpiod -o assignment2

clean:
	rm -f *.o
	rm -f Module.symvers
	rm -f modules.order
	rm -f *.mod.c
	rm -rf .tmp_versions
	rm -f *.mod.c
	rm -f *.mod.o
	rm -f \.*.cmd
	rm -f Module.markers
