# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Ethernet Xaui build constituents
# Author:      S. Maldonado <smaldona@slac.stanford.edu>
# Created:     10/30/2014
# Credits:     SLAC
#                              Copyright 2014
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

ifeq ($(have_arm),$(true))
ifeq ($(have_rtems),$(true))


# When this macro is defined the BSD-related headers reveal
# definitions private to the BSD network stack. The name of the macro
# may change in future RTEMS releases but for now (RTEMS 4.9, 4.10)
# this works.  See rtems/rtems_bsdnet.h.
DEFINES += -D__INSIDE_RTEMS_BSD_TCPIP_STACK__

modnames          := ethXaui
modsrcs_ethXaui   := src/ethXaui.c
modsrcs_ethXaui   += src/ethXaui_dma.c
modsrcs_ethXaui   += src/ethXaui_hw.c
modsrcs_ethXaui   += src/ethernet.c
modlibs_ethXaui   := task/taskstub
basetgt_ethXaui   := platform/rtems
basemod_ethXaui   := driver/network
basemod_ethXaui   += ppi/xaui
endif
endif
