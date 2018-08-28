# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the Xilinx Built-In Self Test.
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     2/13/2013
# Commited:   \$Date: 2014-11-05 12:18:04 -0800 (Wed, 05 Nov 2014) $ by \$Author: panetta $.
# Revision:   \$Revision: 3814 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/xilinx/bist/constituents.mk $
# Credits:     SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

do_not_document = $(true)

# Build only for the ZYNQ without full C (or C++) runtime support.
ifeq ($(and $(have_zynq),$(have_noose)),$(true))
libnames := bist

libsrcs_bist :=

libsrcs_bist += src/dcfg_test.c
libsrcs_bist += src/hello_uart.c
libsrcs_bist += src/interrupt_test.c
libsrcs_bist += src/memory_config_g.c
libsrcs_bist += src/memorytest.c
libsrcs_bist += src/menu.c
libsrcs_bist += src/platform.c
libsrcs_bist += src/scugic_test.c
libsrcs_bist += src/watchdog_test.c
libsrcs_bist += src/xcanps_intr_example.c
libsrcs_bist += src/xdevcfg_selftest_example.c
libsrcs_bist += src/xemacps_example_intr_dma.c
libsrcs_bist += src/xemacps_example_util.c
libsrcs_bist += src/xgpiops_polled_example.c
libsrcs_bist += src/xiicps_eeprom_intr_example.c
libsrcs_bist += src/xiicps_hdmi_intr_example.c
libsrcs_bist += src/xiicps_port_expander_intr_example.c
libsrcs_bist += src/xiicps_rtc_intr_example.c
libsrcs_bist += src/xiicps_si570_intr_example.c
libsrcs_bist += src/xscugic_tapp_example.c
libsrcs_bist += src/xscutimer_intr_example.c
libsrcs_bist += src/xscuwdt_intr_example.c
libsrcs_bist += src/xttcps_intr_example.c
libsrcs_bist += src/xwdtps_selftest_example.c

endif
