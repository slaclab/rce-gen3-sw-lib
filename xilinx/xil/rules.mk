# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        rules.mk
# Abstract:    Special make rules for the Xilinx BSP library
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     2/13/2013
# Committed:  \$Date: 2014-04-07 21:30:13 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.
# Revision:   \$Revision: 3126 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/xilinx/xil/rules.mk $
# Credits:     SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

# Build only for the ZYNQ without full C (or C++) runtime support.
ifeq ($(have_zynq),$(true))

include xilPkgs.mk

CPPFLAGS += -I $(incfarm)/xilinxcfg
CPPFLAGS += $(addprefix -I, $(xilPkgs))

endif
