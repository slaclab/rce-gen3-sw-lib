# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the rts package.
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

modnames    := rts
modsrcs_rts := src/rts.cc
modlibs_rts += service/logger
modlibs_rts += tool/exception
modlibs_rts += tool/concurrency
modlibs_rts += tool/io
modlibs_rts += tool/string
modlibs_rts += tool/conversion
#modlibs_rts += tool/cpu
modlibs_rts += tool/memory
modlibs_rts += tool/time

# required for libconcurrency.a __dso_handle symbol
modslibw_rts := $(GNU_LIBS)/crtbegin.o
modslibw_rts += $(GNU_LIBS)/crtend.o

basetgt_rts := platform/rtems

endif
endif
