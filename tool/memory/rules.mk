# -*-Mode: Makefile;-*-
#
#                               Copyright 2014
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Facility: DAT
# Abstract: Rules to make the Linux kernel modules.
# Author: Stephen Tether <tether@slac.stanford.edu>
# Date created: 2014-08-09
# Last commit: \$Date: 2015-08-30 12:47:06 -0700 (Sun, 30 Aug 2015) $ by \$Author: smaldona $.
# Credits: SLAC


ifeq ($(and $(have_arm),$(have_linux)),$(true))


# Building kernel modules is done using the "kbuild" system that comes
# with the kernel; it isn't compatible with our build system. Build
# the modules using a separate invocation of make. I deliberately
# avoid using $(MAKE) so that all the targets and variables of our
# build system don't leak through to the sub-make.

# Kbuild expects to find a Kbuild or Makfile file in the directory in
# which the modules are built, so we make symbolic links in the module
# build directory for that file. We do that for the source files as
# well in order to keep the Kbuild file simple. VPATH doesn't work.
#
# See Documentation/kbuild/modules.txt in the kernel source tree.

.PHONY: kmod_make kmod_clean

#bin: kmod_make

#clean: kmod_clean

compiler  := ARCH=arm CROSS_COMPILE=arm-xilinx-linux-gnueabi-
linuxroot := /afs/slac/g/cci/package/linux-xlnx
kmod_sources := $(PWD)/armCA9-linux/dmabuf.h $(PWD)/src/armCA9-linux/{Kbuild,dmabuf.c,ocm.c}
kmod_make:
	echo "[CK] {$(tgt_arch)} Running kbuild target 'modules'."
	ln -sf  $(kmod_sources) $(moddir)
	+make -C $(linuxroot) $(compiler) M=$(moddir) modules


kmod_clean:
	echo "[RK] {$(tgt_arch)} Running kbuild target 'clean'."
	+([ -L $(moddir)/Kbuild ] && make -C $(linuxroot) $(compiler) M=$(moddir) clean) || true
	rm -f $(moddir)/{*.[ch],Kbuild}
endif
