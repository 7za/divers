#KDIR  := /home/fred/RtEmbedded/kernels/linux-2.6.27/

KVERS= $(shell uname -r)
KDIR := /lib/modules/$(KVERS)/build/
PWD = $(shell pwd)

obj-m = mod_test.o
mod_test-objs =  modtest.o 


	
modules::
	$(MAKE)  -C $(KDIR) M=$(PWD) modules

clean::
	rm -f *.o *.ko
