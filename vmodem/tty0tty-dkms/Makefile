obj-m := tty0tty.o

KVERSION ?= $(shell uname -r)
KDIR ?= /lib/modules/$(KVERSION)/build
PWD := $(shell pwd)
ARCH ?= $(shell uname -m)
ifeq ($(ARCH),aarch64)
ARCH := arm64
endif

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) clean
