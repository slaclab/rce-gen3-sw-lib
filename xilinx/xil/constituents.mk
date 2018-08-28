# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the Xilinx BSP code.
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     2/13/2014
# Commited:   \$Date: 2014-11-05 12:18:04 -0800 (Wed, 05 Nov 2014) $ by \$Author: panetta $.
# Revision:   \$Revision: 3814 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/xilinx/xil/constituents.mk $
# Credits:     SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

do_not_document = $(true)

# For non-linux platforms, build only for the ZYNQ without full C (or C++)
# runtime support.
ifeq ($(have_linux),$(false))
ifeq ($(have_zynq),$(true))
libnames := xil

libsrcs_xil :=

include xilPkgs.mk

libsrcs_xil += src/$(bram)/src/xbram.c
libsrcs_xil += src/$(bram)/src/xbram_g.c
libsrcs_xil += src/$(bram)/src/xbram_intr.c
libsrcs_xil += src/$(bram)/src/xbram_selftest.c
libsrcs_xil += src/$(bram)/src/xbram_sinit.c

libsrcs_xil += src/$(canps)/src/xcanps.c
libsrcs_xil += src/$(canps)/src/xcanps_g.c
libsrcs_xil += src/$(canps)/src/xcanps_intr.c
libsrcs_xil += src/$(canps)/src/xcanps_selftest.c
libsrcs_xil += src/$(canps)/src/xcanps_sinit.c

libsrcs_xil += src/$(common)/src/xbasic_types.c
libsrcs_xil += src/$(common)/src/xutil_memtest.c
libsrcs_xil += src/$(common)/src/xversion.c

libsrcs_xil += src/$(devcfg)/src/xdevcfg.c
libsrcs_xil += src/$(devcfg)/src/xdevcfg_g.c
libsrcs_xil += src/$(devcfg)/src/xdevcfg_intr.c
libsrcs_xil += src/$(devcfg)/src/xdevcfg_selftest.c
libsrcs_xil += src/$(devcfg)/src/xdevcfg_sinit.c

libsrcs_xil += src/$(dmaps)/src/xdmaps.c
libsrcs_xil += src/$(dmaps)/src/xdmaps_g.c
libsrcs_xil += src/$(dmaps)/src/xdmaps_selftest.c
libsrcs_xil += src/$(dmaps)/src/xdmaps_sinit.c

libsrcs_xil += src/$(emacps)/src/xemacps.c
libsrcs_xil += src/$(emacps)/src/xemacps_bdring.c
libsrcs_xil += src/$(emacps)/src/xemacps_control.c
libsrcs_xil += src/$(emacps)/src/xemacps_g.c
libsrcs_xil += src/$(emacps)/src/xemacps_intr.c
libsrcs_xil += src/$(emacps)/src/xemacps_sinit.c

libsrcs_xil += src/$(gpiops)/src/xgpiops.c
libsrcs_xil += src/$(gpiops)/src/xgpiops_g.c
libsrcs_xil += src/$(gpiops)/src/xgpiops_intr.c
libsrcs_xil += src/$(gpiops)/src/xgpiops_selftest.c
libsrcs_xil += src/$(gpiops)/src/xgpiops_sinit.c

libsrcs_xil += src/$(iicps)/src/xiicps.c
libsrcs_xil += src/$(iicps)/src/xiicps_g.c
libsrcs_xil += src/$(iicps)/src/xiicps_intr.c
libsrcs_xil += src/$(iicps)/src/xiicps_master.c
libsrcs_xil += src/$(iicps)/src/xiicps_options.c
libsrcs_xil += src/$(iicps)/src/xiicps_selftest.c
libsrcs_xil += src/$(iicps)/src/xiicps_sinit.c
libsrcs_xil += src/$(iicps)/src/xiicps_slave.c

libsrcs_xil += src/$(qspips)/src/xqspips.c
libsrcs_xil += src/$(qspips)/src/xqspips_g.c
libsrcs_xil += src/$(qspips)/src/xqspips_options.c
libsrcs_xil += src/$(qspips)/src/xqspips_selftest.c
libsrcs_xil += src/$(qspips)/src/xqspips_sinit.c

libsrcs_xil += src/$(scugic)/src/xscugic.c
libsrcs_xil += src/$(scugic)/src/xscugic_g.c
libsrcs_xil += src/$(scugic)/src/xscugic_hw.c
libsrcs_xil += src/$(scugic)/src/xscugic_intr.c
libsrcs_xil += src/$(scugic)/src/xscugic_selftest.c
libsrcs_xil += src/$(scugic)/src/xscugic_sinit.c

libsrcs_xil += src/$(scutimer)/src/xscutimer.c
libsrcs_xil += src/$(scutimer)/src/xscutimer_g.c
libsrcs_xil += src/$(scutimer)/src/xscutimer_selftest.c
libsrcs_xil += src/$(scutimer)/src/xscutimer_sinit.c

libsrcs_xil += src/$(scuwdt)/src/xscuwdt.c
libsrcs_xil += src/$(scuwdt)/src/xscuwdt_g.c
libsrcs_xil += src/$(scuwdt)/src/xscuwdt_selftest.c
libsrcs_xil += src/$(scuwdt)/src/xscuwdt_sinit.c

ifeq ($(have_rtems),$(false))
libsrcs_xil += src/$(standalone)/src/_exit.c
libsrcs_xil += src/$(standalone)/src/_open.c
libsrcs_xil += src/$(standalone)/src/_sbrk.c
libsrcs_xil += src/$(standalone)/src/abort.c
libsrcs_xil += src/$(standalone)/src/asm_vectors.S
libsrcs_xil += src/$(standalone)/src/boot.S
libsrcs_xil += src/$(standalone)/src/close.c
libsrcs_xil += src/$(standalone)/src/cpu_init.S
libsrcs_xil += src/$(standalone)/src/errno.c
libsrcs_xil += src/$(standalone)/src/fcntl.c
libsrcs_xil += src/$(standalone)/src/fstat.c
libsrcs_xil += src/$(standalone)/src/getpid.c
libsrcs_xil += src/$(standalone)/src/inbyte.c
libsrcs_xil += src/$(standalone)/src/isatty.c
libsrcs_xil += src/$(standalone)/src/kill.c
libsrcs_xil += src/$(standalone)/src/lseek.c
libsrcs_xil += src/$(standalone)/src/open.c
libsrcs_xil += src/$(standalone)/src/outbyte.c
libsrcs_xil += src/$(standalone)/src/print.c
libsrcs_xil += src/$(standalone)/src/putnum.c
libsrcs_xil += src/$(standalone)/src/read.c
libsrcs_xil += src/$(standalone)/src/sbrk.c
libsrcs_xil += src/$(standalone)/src/sleep.c
libsrcs_xil += src/$(standalone)/src/smc.c
libsrcs_xil += src/$(standalone)/src/translation_table.s
libsrcs_xil += src/$(standalone)/src/uart.c
libsrcs_xil += src/$(standalone)/src/unlink.c
libsrcs_xil += src/$(standalone)/src/usleep.c
libsrcs_xil += src/$(standalone)/src/vectors.c
libsrcs_xil += src/$(standalone)/src/write.c
libsrcs_xil += src/$(standalone)/src/xil-crt0.S
libsrcs_xil += src/$(standalone)/src/xil_testcache.c
libsrcs_xil += src/$(standalone)/src/xil_testio.c
libsrcs_xil += src/$(standalone)/src/xil_testmem.c
libsrcs_xil += src/$(standalone)/src/xl2cc_counter.c
libsrcs_xil += src/$(standalone)/src/xpm_counter.c
libsrcs_xil += src/$(standalone)/src/xtime_l.c

#libsrcs_xil += src/$(standalone)/src/profile/_profile_clean.c
#libsrcs_xil += src/$(standalone)/src/profile/_profile_init.c
#libsrcs_xil += src/$(standalone)/src/profile/_profile_timer_hw.c
#libsrcs_xil += src/$(standalone)/src/profile/dummy.S
#libsrcs_xil += src/$(standalone)/src/profile/profile_cg.c
#libsrcs_xil += src/$(standalone)/src/profile/profile_hist.c
#libsrcs_xil += src/$(standalone)/src/profile/profile_mcount_arm.S
#libsrcs_xil += src/$(standalone)/src/profile/profile_mcount_mb.S
#libsrcs_xil += src/$(standalone)/src/profile/profile_mcount_ppc.S
endif

# These should be replaced with RTEMS equivalents when compiling for RTEMS
libsrcs_xil += src/$(standalone)/src/xil_assert.c
libsrcs_xil += src/$(standalone)/src/xil_cache.c
libsrcs_xil += src/$(standalone)/src/xil_exception.c
libsrcs_xil += src/$(standalone)/src/xil_io.c
libsrcs_xil += src/$(standalone)/src/xil_mmu.c

ifeq ($(have_rtems),$(false))
libsrcs_xil += src/$(standalone)/src/xil_printf.c
else
libsrcs_xil += src/xil_printf.c
endif

libsrcs_xil += src/$(ttcps)/src/xttcps.c
libsrcs_xil += src/$(ttcps)/src/xttcps_g.c
libsrcs_xil += src/$(ttcps)/src/xttcps_options.c
libsrcs_xil += src/$(ttcps)/src/xttcps_selftest.c
libsrcs_xil += src/$(ttcps)/src/xttcps_sinit.c

libsrcs_xil += src/$(uartps)/src/xuartps.c
libsrcs_xil += src/$(uartps)/src/xuartps_g.c
libsrcs_xil += src/$(uartps)/src/xuartps_hw.c
libsrcs_xil += src/$(uartps)/src/xuartps_intr.c
libsrcs_xil += src/$(uartps)/src/xuartps_options.c
libsrcs_xil += src/$(uartps)/src/xuartps_selftest.c
libsrcs_xil += src/$(uartps)/src/xuartps_sinit.c

libsrcs_xil += src/$(usbps)/src/xusbps.c
libsrcs_xil += src/$(usbps)/src/xusbps_endpoint.c
libsrcs_xil += src/$(usbps)/src/xusbps_g.c
libsrcs_xil += src/$(usbps)/src/xusbps_intr.c
libsrcs_xil += src/$(usbps)/src/xusbps_sinit.c

libsrcs_xil += src/$(wdtps)/src/xwdtps.c
libsrcs_xil += src/$(wdtps)/src/xwdtps_g.c
libsrcs_xil += src/$(wdtps)/src/xwdtps_selftest.c
libsrcs_xil += src/$(wdtps)/src/xwdtps_sinit.c

libsrcs_xil += src/$(xadcps)/src/xadcps.c
libsrcs_xil += src/$(xadcps)/src/xadcps_g.c
libsrcs_xil += src/$(xadcps)/src/xadcps_intr.c
libsrcs_xil += src/$(xadcps)/src/xadcps_selftest.c
libsrcs_xil += src/$(xadcps)/src/xadcps_sinit.c

endif
endif
