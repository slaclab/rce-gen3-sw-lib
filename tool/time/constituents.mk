# -*-Mode: Makefile;-*-
#                               Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Abstract: constituents.mk for tool/time
# Author: Jim Panetta  ---  panetta@slac.stanford.edu
# Date created: 2011/04/20
# Last commit: \$Date: 2014-12-11 14:19:55 -0800 (Thu, 11 Dec 2014) $ by \$Author: panetta $.
# Revision number: \$Revision: 3844 $
# Location in repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/time/constituents.mk $
# Credits: SLAC

libnames := 
tasknames :=
tgtnames :=

# The "time" library.
# Time.cc requires POSIX and C library routines but is otherwise
# platform independent.
ifeq ($(have_noose),$(false))
libnames += time
libsrcs_time := src/Time.cc
endif
# Manipulation of Zynq timing hardware. Needs no OS support unless
# debugging printout is uncommented.
ifeq ($(and $(have_rtems),$(have_armCA9)),$(true))
ifeq ($(or $(have_noose),$(have_rtems)),$(true))
libsrcs_time += src/armCA9/tripleTimer.c
libsrcs_time += src/armCA9/clockRate.c
endif
endif


# RTEMS test code for the triple-timer API.
ifeq ($(have_armCA9),$(true))
ifeq ($(have_rtems),$(true))
tasknames += testTtc
tasksrcs_testTtc := test/armCA9-rtems/testTtc.cc
basetgt_testTtc  := platform/rtems
basemod_testTtc  := platform/rts
endif
endif

# RTEMS test code for the clock-rate calculator.
ifeq ($(have_armCA9),$(true))
ifeq ($(have_rtems),$(true))
tasknames += testClockRate
tasksrcs_testClockRate := test/armCA9-rtems/testClockRate.cc
basetgt_testClockRate := platform/rtems
endif
endif

# RTEMS code used to time various operations.
ifeq ($(and $(have_rtems),$(have_armCA9)),$(true))
tasknames += codeTiming
tasksrcs_codeTiming :=
tasksrcs_codeTiming += test/armCA9-rtems/codeTiming.cc      # Main program.
tasksrcs_codeTiming += test/armCA9-rtems/callTimes.cc       # Various kinds of function calls.
tasksrcs_codeTiming += test/armCA9-rtems/yieldTimes.cc      # Context switches.
tasksrcs_codeTiming += test/armCA9-rtems/memrefTimes.cc     # NOPs and cached memory references.
tasksrcs_codeTiming += test/armCA9-rtems/memopTimes.cc      # Cache flushing and MMU table walking.
basetgt_codeTiming  := platform/rtems
basemod_codeTiming  := platform/rts
endif
