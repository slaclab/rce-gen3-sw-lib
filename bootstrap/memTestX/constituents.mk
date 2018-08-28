# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the Xilinx Memory Test.
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     12/8/2011
# Commited:   \$Date: 2014-11-05 12:17:47 -0800 (Wed, 05 Nov 2014) $ by \$Author: panetta $.
# Revision:   \$Revision: 3813 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/bootstrap/memTestX/constituents.mk $
# Credits:     SLAC
#                              Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

do_not_document = $(true)

# Build only for the PPC based RCEs without full C (or C++) runtime support.
ifeq ($(and $(have_rce),$(have_ppc)),$(true))
ifeq ($(have_std_c),$(false))
tgtnames := memTestX

tgtsrcs_memTestX := src/cpu_init.S
tgtsrcs_memTestX += ../common/src/$(tgt_cpu_model)/cpuInit.S
tgtsrcs_memTestX += ../common/src/print.c
tgtsrcs_memTestX += src/TestApp_Memory.c
tgtsrcs_memTestX += src/xil_testmem.c
tgtsrcs_memTestX += src/xil_assert.c
tgtsrcs_memTestX += src/xil_io.c

endif
endif

