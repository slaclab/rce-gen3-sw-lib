# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for our Zynq First Stage Boot Loader.
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     2/13/2013
# Commited:   \$Date: 2014-11-05 12:17:47 -0800 (Wed, 05 Nov 2014) $ by \$Author: panetta $.
# Revision:   \$Revision: 3813 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/bootstrap/fsbl/constituents.mk $
# Credits:     SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

do_not_document = $(true)

# We need to build with RTEMS tools although the result can be
# used on non-RTEMS platforms.
ifeq ($(and $(have_zynq),$(have_noose)),$(true))

include $(RELEASE_DIR)/xilinx/xil/xilPkgs.mk

cfgDir  := $(RELEASE_DIR)/configuration
xilDir  := $(xilPrj)/xil/src
fsblDir := $(xilPrj)/fsbl/src
bistDir := $(xilPrj)/bist/src

tgtnames := fsbl bist memTest

# FSBL:

# Commented out because although using libxil.a works, too, it results in the
# entire contents of the archive being included in the executable.  To keep
# size down, better to cherry pick what we need below.
#tgtlibs_fsbl := xilinx/xil
tgtlibs_fsbl := xilinx/fsbl

tgtsrcs_fsbl :=

tgtsrcs_fsbl += $(fsblDir)/fsbl_hooks.c
tgtsrcs_fsbl += $(cfgDir)/xilinx/src/$(tgt_board)/ps7_init.c

# Explicitly list these here to avoid including the whole archive in the exe
tgtsrcs_fsbl += $(xilDir)/$(devcfg)/src/xdevcfg.c
tgtsrcs_fsbl += $(xilDir)/$(devcfg)/src/xdevcfg_g.c
tgtsrcs_fsbl += $(xilDir)/$(devcfg)/src/xdevcfg_intr.c
tgtsrcs_fsbl += $(xilDir)/$(devcfg)/src/xdevcfg_sinit.c

tgtsrcs_fsbl += $(xilDir)/$(qspips)/src/xqspips.c
tgtsrcs_fsbl += $(xilDir)/$(qspips)/src/xqspips_g.c
tgtsrcs_fsbl += $(xilDir)/$(qspips)/src/xqspips_options.c
tgtsrcs_fsbl += $(xilDir)/$(qspips)/src/xqspips_selftest.c
tgtsrcs_fsbl += $(xilDir)/$(qspips)/src/xqspips_sinit.c

tgtsrcs_fsbl += $(xilDir)/$(standalone)/src/_exit.c
tgtsrcs_fsbl += $(xilDir)/$(standalone)/src/_sbrk.c
tgtsrcs_fsbl += $(xilDir)/$(standalone)/src/asm_vectors.S
tgtsrcs_fsbl += $(xilDir)/$(standalone)/src/boot.S
tgtsrcs_fsbl += $(xilDir)/$(standalone)/src/cpu_init.S
tgtsrcs_fsbl += $(xilDir)/$(standalone)/src/outbyte.c
tgtsrcs_fsbl += $(xilDir)/$(standalone)/src/translation_table.s
tgtsrcs_fsbl += $(xilDir)/$(standalone)/src/usleep.c
tgtsrcs_fsbl += $(xilDir)/$(standalone)/src/vectors.c
tgtsrcs_fsbl += $(xilDir)/$(standalone)/src/xtime_l.c
tgtsrcs_fsbl += $(xilDir)/$(standalone)/src/xil-crt0.S
tgtsrcs_fsbl += $(xilDir)/$(standalone)/src/xil_assert.S
tgtsrcs_fsbl += $(xilDir)/$(standalone)/src/xil_cache.c
tgtsrcs_fsbl += $(xilDir)/$(standalone)/src/xil_exception.c
tgtsrcs_fsbl += $(xilDir)/$(standalone)/src/xil_io.c
tgtsrcs_fsbl += $(xilDir)/$(standalone)/src/xil_mmu.c
tgtsrcs_fsbl += $(xilDir)/$(standalone)/src/xil_printf.c

tgtsrcs_fsbl += $(xilDir)/$(uartps)/src/xuartps.c
tgtsrcs_fsbl += $(xilDir)/$(uartps)/src/xuartps_g.c
tgtsrcs_fsbl += $(xilDir)/$(uartps)/src/xuartps_hw.c
tgtsrcs_fsbl += $(xilDir)/$(uartps)/src/xuartps_intr.c
tgtsrcs_fsbl += $(xilDir)/$(uartps)/src/xuartps_options.c
tgtsrcs_fsbl += $(xilDir)/$(uartps)/src/xuartps_sinit.c

tgtsrcs_fsbl += $(xilDir)/$(wdtps)/src/xwdtps.c
tgtsrcs_fsbl += $(xilDir)/$(wdtps)/src/xwdtps_g.c
tgtsrcs_fsbl += $(xilDir)/$(wdtps)/src/xwdtps_sinit.c


# bist
tgtlibs_bist := xilinx/xil
tgtlibs_bist += xilinx/bist

tgtsrcs_bist :=

# memTest:
tgtsrcs_memTest :=

tgtsrcs_memTest += src/memTest.c
tgtsrcs_memTest += src/memory_config.c
tgtsrcs_memTest += src/memorytest.c
tgtsrcs_memTest += $(fsblDir)/ddr_init.c
tgtsrcs_memTest += $(cfgDir)/xilinx/src/$(tgt_board)/ps7_init.c
tgtsrcs_memTest += $(bistDir)/platform.c

tgtsrcs_memTest += $(xilDir)/$(standalone)/src/_exit.c
tgtsrcs_memTest += $(xilDir)/$(standalone)/src/_sbrk.c
tgtsrcs_memTest += $(xilDir)/$(standalone)/src/asm_vectors.S
tgtsrcs_memTest += $(xilDir)/$(standalone)/src/boot.S
tgtsrcs_memTest += $(xilDir)/$(standalone)/src/cpu_init.S
tgtsrcs_memTest += $(xilDir)/$(standalone)/src/outbyte.c
tgtsrcs_memTest += $(xilDir)/$(standalone)/src/print.c
tgtsrcs_memTest += $(xilDir)/$(standalone)/src/putnum.c
tgtsrcs_memTest += $(xilDir)/$(standalone)/src/translation_table.s
tgtsrcs_memTest += $(xilDir)/$(standalone)/src/vectors.c
tgtsrcs_memTest += $(xilDir)/$(standalone)/src/xtime_l.c
tgtsrcs_memTest += $(xilDir)/$(standalone)/src/xil-crt0.S
tgtsrcs_memTest += $(xilDir)/$(standalone)/src/xil_assert.S
tgtsrcs_memTest += $(xilDir)/$(standalone)/src/xil_cache.c
tgtsrcs_memTest += $(xilDir)/$(standalone)/src/xil_exception.c
tgtsrcs_memTest += $(xilDir)/$(standalone)/src/xil_io.c
tgtsrcs_memTest += $(xilDir)/$(standalone)/src/xil_mmu.c
tgtsrcs_memTest += $(xilDir)/$(standalone)/src/xil_printf.c

tgtsrcs_memTest += $(xilDir)/$(uartps)/src/xuartps.c
tgtsrcs_memTest += $(xilDir)/$(uartps)/src/xuartps_g.c
tgtsrcs_memTest += $(xilDir)/$(uartps)/src/xuartps_hw.c
tgtsrcs_memTest += $(xilDir)/$(uartps)/src/xuartps_intr.c
tgtsrcs_memTest += $(xilDir)/$(uartps)/src/xuartps_options.c
tgtsrcs_memTest += $(xilDir)/$(uartps)/src/xuartps_sinit.c

tgtsrcs_memTest += $(xilDir)/$(standalone)/src/xil_testmem.c

endif
