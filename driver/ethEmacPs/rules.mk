# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        rules.mk
# Abstract:    Ethernet EmacPS build rules
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     8/13/2013
# Committed:  \$Date: 2015-01-05 16:15:05 -0800 (Mon, 05 Jan 2015) $ by \$Author: panetta $.
# Revision:   \$Revision: 3919 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/driver/ethEmacPs/rules.mk $
# Credits:     SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
ifeq ($(and $(have_rtems),$(have_zynq)),$(true))

include $(RELEASE_DIR)/xilinx/xil/xilPkgs.mk

#CPPFLAGS += -I$(PWD)
CPPFLAGS += -I$(incfarm)/xilinxcfg
CPPFLAGS += $(addprefix -I, $(xilPkgs))

endif
