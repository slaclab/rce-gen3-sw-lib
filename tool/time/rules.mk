# -*-Mode: Makefile;-*-
#
#                               Copyright 2014
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Facility: DAT
# Abstract: Get access to Xilinx headers.
# Author: Stephen Tether <tether@slac.stanford.edu>
# Date created: 2014/06/09
# Credits: SLAC

# The TTC test task uses the Xilinx support routines for the Zynq TTC
# and so to compile it we need access to Xilinx headers.

ifeq ($(have_armCA9),$(true))
ifeq ($(or $(have_noose),$(have_rtems)),$(true))

# Get the list of packages in the "xilinx" project.
include $(RELEASE_DIR)/xilinx/xil/xilPkgs.mk

# Search for headers in our own configuration header directory then in
# each of the Xilinx projects.
CPPFLAGS += -I$(incfarm)/xilinxcfg
CPPFLAGS += $(addprefix -I, $(xilPkgs))

endif
endif
