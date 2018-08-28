# -*-Mode: Makefile;-*-
#
#                               Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Facility: DAT
# Abstract: Make shared-object files for the linker's unit test.
# Author: Stephen Tether <tether@slac.stanford.edu>
# Date created: 2013/12/02
# Last commit: \$Date: 2014-04-22 18:43:06 -0700 (Tue, 22 Apr 2014) $ by \$Author: panetta $.
# Revision number: \$Revision: 3228 $
# Location in repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/elf/test/linux/rules.mk $
# Credits: SLAC

ifeq ($(have_linux),$(true))
ifeq ($(have_arm),$(true))
ifeq ($(have_64bit),$(false))

objs := one.exe two.so three.so four.so five.so six.so

binobjs := $(foreach ob,$(objs),$(bindir)/$(ob))

CROSS_COMPILE ?= arm-xilinx-linux-
command := $(CROSS_COMPILE)gcc -shared -nostartfiles -nostdlib -Ttest/linux/testobj.ld -Wl,-zcombreloc,--hash-style=gnu,-zmax-page-size=4096,-zdefs -fno-builtin -fPIC -I $(incfarm) -DEXPORT=""

# Append additional dependencies to the global "bin" and "clean" targets.
bin: $(binobjs)

.PHONY: toolelftestclean

clean: toolelftestclean

toolelftestclean:
	echo '[RL] {$(tgt_arch)} Removing $(objs)'
	rm -f $(binobjs)

$(bindir)/one.exe: test/linux/one.c $(bindir)/four.so $(bindir)/two.so $(bindir)/three.so
	echo '[CC] {$(tgt_arch)} Compiling $< into shared object.'
	$(command) -o $@ $^ -Wl,-soname,one

$(bindir)/two.so: test/linux/two.c $(bindir)/four.so
	echo '[CC] {$(tgt_arch)} Compiling $< into shared object.'
	$(command) -o $@ $^  -Wl,-soname,two

$(bindir)/three.so: test/linux/three.c $(bindir)/four.so
	echo '[CC] {$(tgt_arch)} Compiling $< into shared object.'
	$(command) -o $@ $^  -Wl,-soname,three

$(bindir)/four.so: test/linux/four.c $(bindir)/five.so
	echo '[CC] {$(tgt_arch)} Compiling $< into shared object.'
	$(command) -o $@ $^ -Wl,-soname,four

$(bindir)/five.so: test/linux/five.c
	echo '[CC] {$(tgt_arch)} Compiling $< into shared object.'
	$(command) -o $@ $^ -Wl,-soname,five

$(bindir)/six.so: test/linux/six.c $(bindir)/four.so
	echo '[CC] {$(tgt_arch)} Compiling $< into shared object.'
	$(command) -o $@ $^ -Wl,-soname,six
endif
endif
endif
