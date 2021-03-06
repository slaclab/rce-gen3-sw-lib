# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for startup/memTest.
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     9/30/2011
# Committed:  \$Date: 2014-11-05 12:17:47 -0800 (Wed, 05 Nov 2014) $ by \$Author: panetta $.
# Revision:   \$Revision: 3813 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/bootstrap/memTest/constituents.mk $
# Credits:     SLAC
#                              Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

do_not_document = $(true)

# Build only for the PPC based RCEs
ifeq ($(tgt_cpu_family)-$(tgt_os),ppc-rtems)

libnames := memTest

libsrcs_memTest := ../common/src/boot.S
libsrcs_memTest +=           src/memTest.S
libsrcs_memTest += ../common/src/$(tgt_cpu_model)/cpuInit.S
libsrcs_memTest += ../common/src/bugcheck.S
libsrcs_memTest += ../common/src/memTests.S

ifeq ($(tgt_board),rceG1)

libnames += memTest_old

libsrcs_memTest_old := ../common/src/boot.S
libsrcs_memTest_old += src/$(tgt_gen)/memTest_old.s

endif
endif
