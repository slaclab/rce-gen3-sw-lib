# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the nfs package.
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

modnames     := nfs
modsrcs_nfs  := src/nfs.c src/rpcio.c
modslibw_nfs := $(RTEMS_LIBS)/libnfs.a
basetgt_nfs  := platform/rtems
basemod_nfs  := driver/network

endif
endif
