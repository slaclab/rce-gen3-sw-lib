obj-m += Interrupt.o

ifeq ($(DAT_ROOT),)
  DAT_ROOT=$(RELEASE_DIR)
endif
xilSrc   := $(DAT_ROOT)/xilinx
linuxSrc := /afs/slac/g/cci/package/linux-xlnx

common       := common_v1_00_a
scugic       := scugic_v1_04_a
standalone   := standalone_v3_10_a

_xilPkgs :=

_xilPkgs += $(common)
_xilPkgs += $(scugic)
_xilPkgs += $(standalone)

xilPkgs := $(addprefix $(xilSrc)/xil/src/, $(addsuffix /src, $(_xilPkgs)))

# Add to the global CPPFLAGS or dependency rules don't see the change
# Avoid changing the global LSCRIPT and LXFLAGS
# Note: INCFARM is passed in from the package Makefile
EXTRA_CFLAGS       += -I$(INCFARM)
EXTRA_CFLAGS       += -I$(linuxSrc)/include/linux
EXTRA_CFLAGS       += $(addprefix -I, $(xilPkgs))

all:
	$(MAKE) ARCH=arm CROSS_COMPILE=arm-xilinx-linux-gnueabi- -C $(linuxSrc) M=$(shell pwd) modules

clean:
	$(MAKE) ARCH=arm CROSS_COMPILE=arm-xilinx-linux-gnueabi- -C $(linuxSrc) M=$(shell pwd) clean
