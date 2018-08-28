# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the FPGA package.
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     Thu Oct 17 16:38:39 2013
# Committed:  \$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.
# Revision:   \$Revision: 3125 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/fpga/constituents.mk $
# Credits:     SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
libnames := fpga

libsrcs_fpga :=

ifeq ($(and $(have_rtems),$(have_zynq)),$(true))

include $(RELEASE_DIR)/xilinx/xil/xilPkgs.mk

fsblDir := $(xilPrj)/fsbl/src
cfgDir  := $(RELEASE_DIR)/configuration

#CPPFLAGS += -DFSBL_DEBUG_INFO=DEBUG_INFO
CPPFLAGS += -I$(cfgDir)/xilinxcfg
CPPFLAGS += -I$(fsblDir)
CPPFLAGS += $(addprefix -I, $(xilPkgs))

libsrcs_fpga += src/fpga.c

modnames := load

majorv_load  := 1
minorv_load  := 0
branch_load  := test
modsrcs_load := test/load.cc
modslib_load := gcc

endif
