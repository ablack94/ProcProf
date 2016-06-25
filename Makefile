KERNELDIR ?= /lib/modules/$(shell uname -r)/build

obj-m += prof2.o

PWD := $(shell pwd)

EXTRA_CFLAGS := -I$(PWD)

all:
	make -C $(KERNELDIR) M=$(PWD) modules

