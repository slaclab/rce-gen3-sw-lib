# -*-Mode: Makefile;-*-
#
#                               Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Facility: DAT
# Abstract: Constituents file for the task package.
# Author: Stephen Tether <tether@slac.stanford.edu>
# Date created: 2013/12/12
# Last commit: \$Date: 2013-12-19 10:48:16 -0800 (Thu, 19 Dec 2013) $ by \$Author: tether $.
# Revision number: \$Revision: 2421 $
# Location in repository: \$HeadURL: file:///afs/slac/g/cci/repositories/ctkrepo/tool/trunk/task/constituents.mk $
# Credits: SLAC


ifeq ($(have_arm),$(true))
ifeq ($(have_rtems),$(true))

libnames            := task
libsrcs_task        :=
libsrcs_task        += src/Task.c
docsrcs_task        += src/Task_Config.c

libnames            += taskstub
libsrcs_taskstub    :=
libsrcs_taskstub    += src/Task_Stub.c

doxy_pkg_c_only=$(true)

endif
endif
