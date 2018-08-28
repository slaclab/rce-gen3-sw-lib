# -*-Mode: Makefile;-*-
#
# Facility:     DAT
# File:         rules.mk
# Abstract:     Special make rules for SAS
# Author:       S. Maldonado <smaldona@slac.stanford.edu>
# Created:      6/26/2014
# Credits:      SLAC
#                              Copyright 2014
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

# Build only for the ARM based boards
ifeq ($(have_arm),$(true))

xilSrc     := $(RELEASE_DIR)/xilinx

CPPFLAGS   += -I$(xilSrc)/xil/src/scugic_v1_04_a/src                   
CPPFLAGS   += -I$(xilSrc)/xil/src/standalone_v3_10_a/src               
CPPFLAGS   += -I$(xilSrc)/xil/src/common_v1_00_a/src                   

ifeq ($(archstring),arm-linux-rceCA9)

moddir     := $(RELEASE_DIR)/build/$(tgt_arch)/mod

#libs       += $(moddir)/Interrupt.ko

# Produce the linux kernel module
$(moddir)/Interrupt.ko: src/linux/Interrupt.c
	@echo "[CK] {$(tgt_arch)} Build kernel module Interrupt.ko"
	$(MAKE) -C src/linux INCFARM=$(incfarm)
	cp $(RELEASE_DIR)/service/sas/src/linux/Interrupt.ko $(moddir)
	$(MAKE) -C src/linux clean INCFARM=$(incfarm)

.PHONY: clean_kmod
clean:  clean_kmod

lib: $(libs)

clean: $(cleans)

clean_kmod:
	@echo "[RK] {$(tgt_arch)} Removing $(notdir $(libs))"
	$(quiet)rm -f $(libs) $(moddir)/Interrupt.ko
	$(MAKE) -C src/linux clean
endif
endif
