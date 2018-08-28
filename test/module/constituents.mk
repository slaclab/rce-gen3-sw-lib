# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the module package.
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     4/12/12
# Committed:  \$Date: 2013-08-14 18:28:21 -0700 (Wed, 14 Aug 2013) $ by \$Author: claus $.
# Revision:   \$Revision: 1929 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/test/module/constituents.mk $
# Credits:     SLAC
#                              Copyright 2012
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

ifeq ($(filter rce405 cm405 rce440, $(tgt_board)), $(tgt_board))
modnames := hello noop shell ramdisk

majorv_hello  := 1
minorv_hello  := 0
branch_hello  := test
modsrcs_hello := src/hello.cc

majorv_noop   := 1
minorv_noop   := 0
branch_noop   := test
modsrcs_noop  := src/noOp.cc

majorv_shell  := 1
minorv_shell  := 0
branch_shell  := test
modsrcs_shell := src/shell.cc
modslib_shell := $(RTEMSDIR)/rtems-gdb-stub

majorv_ramdisk  := 1
minorv_ramdisk  := 0
branch_ramdisk  := test
modsrcs_ramdisk := src/ramdisk.cc

endif
