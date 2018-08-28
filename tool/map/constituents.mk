# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the map package.
# Author:      S. Maldonado <smaldona@slac.stanford.edu>
# Created:     07/20/13
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

libnames        := map
libsrcs_map     :=

ifeq ($(have_linux),$(true))

libsrcs_map     += src/linux/Lookup.c

tgtnames :=
tgtnames += MapTest

tgtsrcs_MapTest:= test/MapTest.c

tgtlibs_MapTest := 
tgtlibs_MapTest += tool/map
tgtlibs_MapTest += tool/mem
tgtlibs_MapTest += tool/debug

else

libsrcs_map         += src/Lookup.c

ifeq ($(have_rtems),$(true))

tasksrcs_common     := test/rtems/test_exe.c
 
tasknames           := MapTest
tasksrcs_MapTest    := $(tasksrcs_common) test/MapTest.c
basetgt_MapTest     := platform/rtems
basemod_MapTest     := platform/rts

endif
endif

endif
