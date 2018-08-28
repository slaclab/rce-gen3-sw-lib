# -*-Mode: Makefile;-*-
#                               Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Abstract: constituents.mk for service/shell
# Author: Ric Claus  ---  claus@slac.stanford.edu
# Date created: 2011/04/25
# Last commit: \$Date: 2015-07-14 10:55:53 -0700 (Tue, 14 Jul 2015) $ by \$Author: smaldona $.
# Revision number: \$Revision: 4078 $
# Location in repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/service/shell/constituents.mk $
# Credits: SLAC


# rce level images
ifeq ($(have_rtems),$(true))

modnames += shell
modslibw_shell := $(RTEMS_LIBS)/libshell.a $(RTEMS_LIBS)/libmonitor.a $(RTEMS_LIBS)/libcapture.a
basetgt_shell  := platform/rtems
basemod_shell  := platform/rts

modsrcs_shell :=  src/rtems/shell.c
modsrcs_shell +=  src/rtems/customizations.cc
modsrcs_shell +=  src/rtems/ShellCommon.c
modsrcs_shell +=  src/rtems/Command.c
modsrcs_shell +=  src/rtems/bootmode.c
modsrcs_shell +=  src/rtems/bootdump.c

# Shell Commands
modsrcs_shell +=  src/rtems/reboot.cc
modsrcs_shell +=  src/rtems/lookup.cc
modsrcs_shell +=  src/rtems/runTask.cc
modsrcs_shell +=  src/rtems/stopTask.cc
modsrcs_shell +=  src/rtems/suspendTask.cc
modsrcs_shell +=  src/rtems/resumeTask.cc
modsrcs_shell +=  src/rtems/syslog.cc
modsrcs_shell +=  src/rtems/resUse.cc
modsrcs_shell +=  src/rtems/nsAssign.cc
modsrcs_shell +=  src/rtems/nsMap.cc
modsrcs_shell +=  src/rtems/nsRemove.cc
modsrcs_shell +=  src/rtems/nsRename.cc
modsrcs_shell +=  src/rtems/loadShareable.cc
modsrcs_shell +=  src/rtems/sysInfo.cc
modsrcs_shell +=  src/rtems/stackspace.cc

modnames += shellx
modsrcs_shellx := src/rtems/extensions.c
# Special for shellx .o files in RTEMS
modslibw_shellx := $(RTEMS_LIBS)/libshell_a-main_ifconfig.o
modslibw_shellx += $(RTEMS_LIBS)/libshell_a-main_netstats.o
modslibw_shellx += $(RTEMS_LIBS)/libshell_a-main_route.o
modslibw_shellx += $(RTEMS_LIBS)/libshell_a-main_ping.o
basetgt_shellx := platform/rtems
basemod_shellx := service/shell
basemod_shellx += driver/network

tasknames := hello
tasksrcs_hello := test/hello_task.cc
basetgt_hello  := platform/rtems

tasknames += redirect
tasksrcs_redirect := test/redirect_task.cc
basetgt_redirect  := platform/rtems

endif
