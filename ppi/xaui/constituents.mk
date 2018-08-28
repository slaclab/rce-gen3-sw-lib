# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the XAUI package.
# Author:      S. Maldonado <smaldona@slac.stanford.edu>
# Created:     08/20/14
# Committed:  \$$ by \$Author:$.
# Revision:   \$Revision:$
# Repository: \$HeadURL:$
# Credits:     SLAC
#                              Copyright 2014
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

ifeq ($(have_arm),$(true))
ifeq ($(have_rtems),$(true))

modnames            := xaui
modsrcs_xaui        := src/Xaui.c src/Xaui_Transmit.c src/Xaui_Access.c src/Xaui_Dump.c
modlibs_xaui        := task/taskstub
basetgt_xaui        := platform/rtems
basemod_xaui        := service/shell
basemod_xaui        += service/sas

modnames            += xaui_test
modsrcs_xaui_test   := test/Xaui_Test.c
basetgt_xaui_test   := platform/rtems
basemod_xaui_test   := service/sas
basemod_xaui_test   += service/shell
basemod_xaui_test   += driver/network
basemod_xaui_test   += ppi/xaui

#modnames            += udp_test
modsrcs_udp_test   := test/Udp_Test.c
basetgt_udp_test   := platform/rtems
basemod_udp_test   += service/shell
basemod_udp_test   += driver/network
basemod_udp_test   += ppi/xaui

endif
endif
