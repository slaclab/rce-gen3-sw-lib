# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the APU package.
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     4/3/12
# Committed:  \$Date: 2014-12-11 14:19:55 -0800 (Thu, 11 Dec 2014) $ by \$Author: panetta $.
# Revision:   \$Revision: 3844 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/cpu/constituents.mk $
# Credits:     SLAC
#                              Copyright 2012
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
ifeq ($(and $(or $(have_rce),$(have_cm)),$(have_cob)),$(true))

#libnames := cpu

endif
