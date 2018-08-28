# -*-Mode: Makefile;-*-
#
# Facility:     DAT
# File:         rules.mk
# Abstract:     Special make rules for the cm boot code
# Author:       S. Maldonado <smaldona@slac.stanford.edu>
# Created:      8/11/2013
# Committed:  \$Date: 2013-05-23 13:16:06 -0700 (Thu, 23 May 2013) $ by \$Author: claus $
# Revision:   \$Revision: 1702 $
# Repository: \$HeadURL: file:///afs/slac/g/cci/repositories/ctkrepo/bootstrap/trunk/fsbl/rules.mk $
# Credits:      SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

# Build only for the ARM based boards
ifeq ($(have_arm),$(true))
# Build only for no operating system (U-Boot)
ifeq ($(have_noose),$(true))
CPPFLAGS  += -ffixed-r8 -mabi=aapcs-linux
endif
endif
