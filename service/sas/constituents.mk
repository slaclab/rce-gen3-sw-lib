# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the SAS package.
# Author:      S. Maldonado <smaldona@slac.stanford.edu>
# Created:     06/02/14
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
srcs_common     := src/Sas.c src/Socket.c src/Mailbox.c src/Interrupt.c src/Transaction.c

ifeq ($(have_linux),$(true))
libnames        := sas
libsrcs_sas     := $(srcs_common)
libsrcs_sas     += src/linux/Sas.c src/linux/Interrupt.c

tgtlibs  :=
tgtlibs  += tool/map
tgtlibs  += tool/mem
tgtlibs  += tool/debug
tgtlibs  += tool/sas
tgtlibs  += tool/kvt
tgtlibs  += tool/hash

tgtnames        :=
tgtnames        += SasTest
tgtsrcs_SasTest := test/SasTest.c
tgtlibs_SasTest := $(tgtlibs)

endif

ifeq ($(have_rtems),$(true))
srcs_rtems          := src/rtems/Interrupt.c
srcs_rtems          += src/rtems/Sas.c src/rtems/Command.c
modnames            := sas
modsrcs_sas         := $(srcs_common)
modsrcs_sas         += $(srcs_rtems)
basetgt_sas         := platform/rtems
basemod_sas         := service/shell

tasksrcs_common     := test/rtems/TestExe.c

tasknames           := SasTest
tasksrcs_SasTest    := $(tasksrcs_common) test/SasTest.c
basetgt_SasTest     := platform/rtems
basemod_SasTest     := service/sas

endif

endif
