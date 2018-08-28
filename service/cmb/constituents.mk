# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the CMB package.
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     3/16/12
# Committed:  \$Date: 2014-02-12 13:34:56 -0800 (Wed, 12 Feb 2014) $ by \$Author: panetta $.
# Revision:   \$Revision: 2772 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/service/cmb/constituents.mk $
# Credits:     SLAC
#                              Copyright 2012
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

libnames := cmb

ifeq ($(and $(or $(have_rce),$(have_cm)),$(have_cob)),$(true))

libsrcs_cmb :=

# There is no BSI on gen 1 boards
ifneq ($(tgt_gen),gen1)
libsrcs_cmb += src/$(tgt_gen)/bsi.cc
endif

endif
