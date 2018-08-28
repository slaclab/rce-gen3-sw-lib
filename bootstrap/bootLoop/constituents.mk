# -*-Mode: Makefile;-*-
#
# Facility:     DAT
# File:         constituents.mk
# Abstract:     Constituents file for bootLoop.
# Author:       R. Claus <claus@slac.stanford.edu>
# Created:      10/27/2011
# Committed:  \$Date: 2014-11-05 12:17:47 -0800 (Wed, 05 Nov 2014) $ by \$Author: panetta $.
# Revision:   \$Revision: 3813 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/bootstrap/bootLoop/constituents.mk $
# Credits:      SLAC
#                              Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

do_not_document=$(true)

# Compile only for RCEs
ifeq ($(and $(have_rtems),$(have_ppc)),$(true))

libnames := bootLoop

libsrcs_bootLoop := ../common/src/boot.S
libsrcs_bootLoop += ../common/src/bugcheck.S
libsrcs_bootLoop += src/bootLoop.S
endif
