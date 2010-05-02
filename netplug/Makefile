obj-m := netplug.o
netplug-objs :=  net_plug.o


KVERS := $(shell uname -r)
KDIR = /lib/modules/$(KVERS)/build
PWD := $(shell pwd)

all::
	$(MAKE)  -C $(KDIR) M=$(PWD) $@

modules::
	$(MAKE)  -C $(KDIR) M=$(PWD) $@
