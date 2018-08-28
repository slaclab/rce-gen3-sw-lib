# -*-Mode: Makefile;-*-
#                               Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Abstract: constituents.mk for tool/debug
# Author: Stephen Tether  ---  tether@slac.stanford.edu
# Date created: 2013/07/31
# Last commit: \$Date: 2014-07-31 17:05:57 -0700 (Thu, 31 Jul 2014) $ by \$Author: tether $.
# Revision number: \$Revision: 3597 $
# Location in repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/debug/constituents.mk $
# Credits: SLAC

libnames := debug
libsrcs_debug :=

tasknames :=

ifeq ($(have_arm),$(true))
libsrcs_debug += src/arm/registers_new.c
endif

ifeq ($(have_rtems),$(true))
libsrcs_debug += src/rtems/print.c
libsrcs_debug += src/rtems/extension_new.c
libsrcs_debug += src/rtems/syslog_new.c
libsrcs_debug += src/rtems/taskExit_new.c
endif

ifeq ($(and $(have_rtems),$(have_arm)),$(true))
libsrcs_debug += src/armCA9-rtems/fatal_new.c
libsrcs_debug += src/armCA9-rtems/init_new.c
endif

ifeq ($(have_linux),$(true))
libsrcs_debug += src/linux/print.c
endif


ifeq ($(and $(have_rtems),$(have_arm)),$(true))
tasknames += bugcheck
tasksrcs_bugcheck := test/armCA9-rtems/bugcheck_task.cc
basetgt_bugcheck  := platform/rtems
endif
