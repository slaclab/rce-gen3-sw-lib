# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        rules.mk
# Abstract:    Build rules for cache test code
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     9/10/2013
# Committed:  \$Date: 2013-09-12 14:54:28 -0700 (Thu, 12 Sep 2013) $ by \$Author: claus $.
# Revision:   \$Revision: 2101 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/test/cache/rules.mk $
# Credits:     SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
ifeq ($(and $(have_rtems),$(have_zynq)),$(true))

include $(RELEASE_DIR)/xilinx/xil/xilPkgs.mk

CPPFLAGS += -I$(RELEASE_DIR)/configuration/xilinx
CPPFLAGS += $(addprefix -I, $(xilPkgs))

endif
