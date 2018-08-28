# -*-Mode: Makefile;-*-
#                               Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Abstract: constituents.mk for tool/exception
# Author: Jim Panetta  ---  panetta@slac.stanford.edu
# Date created: 2011/04/20
# Last commit: \$Date: 2014-03-18 16:15:57 -0700 (Tue, 18 Mar 2014) $ by \$Author: claus $.
# Revision number: \$Revision: 2944 $
# Location in repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/exception/constituents.mk $
# Credits: SLAC

libnames := exception
modnames :=

libsrcs_exception := src/Exception.cc
libsrcs_exception += src/Exceptions.cc
libsrcs_exception += test/testThrow.cc

ifeq ($(have_rtems),$(true))
libsrcs_exception += src/rtems/terminate.cc

tasknames += throw
tasksrcs_throw := test/rtems/throw_task.cc
basetgt_throw  := platform/rtems
basemod_throw  := platform/rts

#modnames += handling
modsrcs_handling := test/handling.cc
majorv_handling  := 1
minorv_handling  := 0
branch_handling  := main
endif


