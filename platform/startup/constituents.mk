# -*-Mode: Makefile;-*-
#
# Facility:     DAT
# File:         constituents.mk
# Abstract:     Constituents file for DBI (DAT Basic Infrastructure)
# Author:       Steve Tether <tether@slac.stanford.edu>
# Created:      2011/05/18
# Committed:  \$Date: 2015-07-14 10:55:53 -0700 (Tue, 14 Jul 2015) $ by \$Author: smaldona $.
# Revision:   \$Revision: 4078 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/platform/startup/constituents.mk $
# Credits:      SLAC
#                              Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.

ifeq ($(and $(or $(have_rce),$(have_cm)),$(have_rtems)),$(true))

tgtnames := rtems.${SHLIBEXTNS}
modnames :=

libs :=
libs += xilinx/xil
libs += driver/Sd
libs += tool/hash
libs += tool/kvt
libs += tool/debug
libs += tool/svt
libs += tool/task
libs += tool/mem
libs += tool/ldr
libs += tool/elf
libs += tool/map
libs += tool/time
libs += tool/cio
libs += tool/pool

srcDir := src

# Production executable
prod_libs := $(libs)

# BSP files that we had to change.
bspOverrides := $(tgt_cpu_family)/bspstarthooks.c $(tgt_cpu_family)/zynq-uart.c $(tgt_cpu_family)/bspgetworkarea.c

tgtsrcs_rtems.${SHLIBEXTNS} := $(addprefix $(srcDir)/, init.c rtemsrefs.c $(bspOverrides))
tgtlibs_rtems.${SHLIBEXTNS} := $(prod_libs)

modnames          += appinit
modsrcs_appinit   := src/appinit.c
basetgt_appinit   := platform/rtems
namespace_appinit := config

endif
