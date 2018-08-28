# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the zynq package.
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     2/5/13
# Committed:  \$Date: 2013-08-14 18:28:21 -0700 (Wed, 14 Aug 2013) $ by \$Author: claus $.
# Revision:   \$Revision: 1929 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/test/zynq/constituents.mk $
# Credits:     SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

tgtnames :=

ifeq ($(have_arm),$(true))
ifeq ($(have_linux),$(true))
tgtnames += hello

tgtsrcs_hello := src/hello.cc
endif

ifeq ($(have_std_c),$(false))
tgtnames += test

tgtsrcs_test := src/test.cc
endif

endif
