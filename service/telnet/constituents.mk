# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the telnet package.
# Author:      S. Maldonado <smaldona@slac.stanford.edu>
# Created:     12/02/13
# Committed:  \$$ by \$Author:$.
# Revision:   \$Revision:$
# Repository: \$HeadURL:$
# Credits:     SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

ifeq ($(have_arm),$(true))
ifeq ($(have_rtems),$(true))
libnames        := telnet_l
libsrcs_telnet_l  := src/telnet.c

modnames += telnet
modlibs_telnet := service/telnet_l
modslibw_telnet := $(RTEMS_LIBS)/libtelnetd.a
basetgt_telnet := platform/rtems
basemod_telnet := service/shell
basemod_telnet += driver/network

endif
endif
