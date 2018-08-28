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
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/bootstrap/testMainApp/constituents.mk $
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
tgtnames := testMainApp

tgtsrcs_testMainApp := src/testMainApp.c
tgtsrcs_testMainApp += src/vectors.S
tgtsrcs_testMainApp += src/cpu_init.S

endif
endif
