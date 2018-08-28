# -*-Mode: Makefile;-*-
#                               Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file.
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     12/12/2011
# Commited:   \$Date: 2014-11-05 12:17:47 -0800 (Wed, 05 Nov 2014) $ by \$Author: panetta $.
# Revision:   \$Revision: 3813 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/bootstrap/testRtemsApp/constituents.mk $
# Credits:     SLAC
#                              Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

do_not_document = $(true)

# Build only for the RCEs with PowerPCs using the RTEMS compiler
ifeq ($(and $(have_rce),$(have_rtems),$(have_ppc)),$(true))

tgtnames := testRtemsApp hello

tgtsrcs_testRtemsApp += src/testRtemsApp.c
tgtsrcs_testRtemsApp += src/testUDI.S

tgtsrcs_hello += src/hello.c

endif
