# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        rules.mk
# Abstract:    SD build rules
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     9/10/2013
# Committed:  \$Date: 2014-04-07 21:29:43 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.
# Revision:   \$Revision: 3121 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/driver/sd/rules.mk $
# Credits:     SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
ifeq ($(and $(have_rtems),$(have_zynq)),$(true))

include $(RELEASE_DIR)/xilinx/xil/xilPkgs.mk

CPPFLAGS += -I$(PWD)
CPPFLAGS += -I$(incfarm)/xilinxcfg
CPPFLAGS += $(addprefix -I, $(xilPkgs))

endif
