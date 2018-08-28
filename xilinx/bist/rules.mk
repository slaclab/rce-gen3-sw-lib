# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        rules.mk
# Abstract:    Special make rules for the Xilinx Built-In Self Test
# Author:      claus <claus@slac.stanford.edu>
# Created:     2/13/2013
# Committed:  \$Date: 2014-04-08 16:06:18 -0700 (Tue, 08 Apr 2014) $ by \$Author: panetta $.
# Revision:   \$Revision: 3131 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/xilinx/bist/rules.mk $
# Credits:     SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

# Build only for the ZYNQ without full C (or C++) runtime support.
ifeq ($(and $(have_zynq),$(have_noose)),$(true))

include $(RELEASE_DIR)/xilinx/xil/xilPkgs.mk

cfgDir := $(RELEASE_DIR)/configuration

CPPFLAGS += -I$(incfarm)/xilinxcfg
CPPFLAGS += $(addprefix -I, $(xilPkgs))

endif
