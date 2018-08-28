# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the boot loader.
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     10/31/2011
# Committed:  \$Date: 2016-05-17 13:05:25 -0700 (Tue, 17 May 2016) $ by \$Author: smaldona $.
# Revision:   \$Revision: 4808 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/bootstrap/bootLoader/constituents.mk $
# Credits:     SLAC
#                              Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

do_not_document = $(true)
no_dummy  = true
NOBLDINFO = true

# We need to build with RTEMS tools although the result can be
# used on non-RTEMS platforms.
ifeq ($(and $(have_rtems),$(have_ppc)),$(true))
libnames := bootLoader
libsrcs_bootLoader += ../common/src/boot.S
libsrcs_bootLoader +=           src/bootLoader.S
libsrcs_bootLoader += ../common/src/$(tgt_cpu_model)/cpuInit.S
libsrcs_bootLoader += ../common/src/$(tgt_cpu_model)/image.S
libsrcs_bootLoader += ../common/src/image.S
libsrcs_bootLoader += ../common/src/elf.S
libsrcs_bootLoader += ../common/src/memTests.S
libsrcs_bootLoader += ../common/src/bugcheck.S
ifeq ($(have_config_flash),$(true))
libsrcs_bootLoader += ../common/src/$(tgt_cpu_model)/flash.S
endif
ifeq ($(have_config_sd),$(true))
libsrcs_bootLoader += ../common/src/$(tgt_cpu_model)/sd.S
endif
endif


# Build only for the ARM based boards
ifeq ($(and $(have_zynq),$(have_noose)),$(true))

include $(RELEASE_DIR)/xilinx/xil/xilPkgs.mk

cfgDir := $(RELEASE_DIR)/configuration

tgtnames := bootLoader

# Commented out because although using libxil.a works, too, it results in the
# entire contents of the archive being included in the executable.  To keep
# size down, better to cherry pick what we need below.
#tgtlibs_bootLoader := xilinx/xil

tgtsrcs_bootLoader :=

# The hooks targeted for our system
tgtsrcs_bootLoader += src/main.c
tgtsrcs_bootLoader += src/gpio.c

tgtsrcs_bootLoader += $(cfgDir)/xilinxcfg/src/$(tgt_board)/ps7_init.c

fsblDir := $(xilPrj)/fsbl/src

# Don't include fsbl_hooks and main as they are to be supplied by this package
tgtsrcs_bootLoader += $(fsblDir)/ddr_init.c
tgtsrcs_bootLoader += $(fsblDir)/ff.c
tgtsrcs_bootLoader += $(fsblDir)/fsbl_hooks.c
tgtsrcs_bootLoader += $(fsblDir)/fsbl_handoff.S
#tgtsrcs_bootLoader += $(fsblDir)/fsbl_hooks.c
tgtsrcs_bootLoader += $(fsblDir)/image_mover.c
#tgtsrcs_bootLoader += $(fsblDir)/main.c
tgtsrcs_bootLoader += $(fsblDir)/md5.c
tgtsrcs_bootLoader += $(fsblDir)/mmc.c
tgtsrcs_bootLoader += $(fsblDir)/nand.c
tgtsrcs_bootLoader += $(fsblDir)/nor.c
tgtsrcs_bootLoader += $(fsblDir)/pcap.c
tgtsrcs_bootLoader += $(fsblDir)/qspi.c
tgtsrcs_bootLoader += $(fsblDir)/sd.c

xilDir := $(xilPrj)/xil/src

# Again, to keep the size down, include only what we need of the Xilinx code.
tgtsrcs_bootLoader += $(xilDir)/$(devcfg)/src/xdevcfg.c
tgtsrcs_bootLoader += $(xilDir)/$(devcfg)/src/xdevcfg_g.c
tgtsrcs_bootLoader += $(xilDir)/$(devcfg)/src/xdevcfg_intr.c
tgtsrcs_bootLoader += $(xilDir)/$(devcfg)/src/xdevcfg_sinit.c

tgtsrcs_bootLoader += $(xilDir)/$(gpiops)/src/xgpiops.c
tgtsrcs_bootLoader += $(xilDir)/$(gpiops)/src/xgpiops_g.c
tgtsrcs_bootLoader += $(xilDir)/$(gpiops)/src/xgpiops_intr.c
tgtsrcs_bootLoader += $(xilDir)/$(gpiops)/src/xgpiops_sinit.c

tgtsrcs_bootLoader += $(xilDir)/$(qspips)/src/xqspips.c
tgtsrcs_bootLoader += $(xilDir)/$(qspips)/src/xqspips_g.c
tgtsrcs_bootLoader += $(xilDir)/$(qspips)/src/xqspips_options.c
tgtsrcs_bootLoader += $(xilDir)/$(qspips)/src/xqspips_selftest.c
tgtsrcs_bootLoader += $(xilDir)/$(qspips)/src/xqspips_sinit.c

tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/close.c
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/fstat.c
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/isatty.c
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/lseek.c
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/read.c
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/write.c
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/inbyte.c
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/_exit.c
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/_sbrk.c
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/asm_vectors.S
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/boot.S
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/cpu_init.S
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/outbyte.c
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/translation_table.s
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/usleep.c
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/vectors.c
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/xtime_l.c
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/xil-crt0.S
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/xil_assert.S
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/xil_cache.c
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/xil_exception.c
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/xil_io.c
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/xil_mmu.c
tgtsrcs_bootLoader += $(xilDir)/$(standalone)/src/xil_printf.c

tgtsrcs_bootLoader += $(xilDir)/$(uartps)/src/xuartps.c
tgtsrcs_bootLoader += $(xilDir)/$(uartps)/src/xuartps_g.c
tgtsrcs_bootLoader += $(xilDir)/$(uartps)/src/xuartps_hw.c
tgtsrcs_bootLoader += $(xilDir)/$(uartps)/src/xuartps_intr.c
tgtsrcs_bootLoader += $(xilDir)/$(uartps)/src/xuartps_options.c
tgtsrcs_bootLoader += $(xilDir)/$(uartps)/src/xuartps_sinit.c

tgtsrcs_bootLoader += $(xilDir)/$(wdtps)/src/xwdtps.c
tgtsrcs_bootLoader += $(xilDir)/$(wdtps)/src/xwdtps_g.c
tgtsrcs_bootLoader += $(xilDir)/$(wdtps)/src/xwdtps_sinit.c

endif
