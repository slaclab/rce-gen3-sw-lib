# -*-Mode: Makefile;-*-
#                               Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Abstract: constituents.mk for tool/io
# Author: Jim Panetta  ---  panetta@slac.stanford.edu
# Date created: 2011/04/20
# Last commit: \$Date: 2014-12-11 14:19:55 -0800 (Thu, 11 Dec 2014) $ by \$Author: panetta $.
# Revision number: \$Revision: 3844 $
# Location in repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/io/constituents.mk $
# Credits: SLAC

# The io library can contain C++ code. The cio library contains only C
# or assembler code and so may be used in rtems.so.

ifeq ($(have_noose),$(false))

libnames    := io cio
libsrcs_io  :=
libsrcs_cio :=

libsrcs_io += src/rwall.cc

ifeq ($(and $(have_arm),$(have_rtems)),$(true))
libsrcs_cio += src/armCA9-rtems/consoleSupport.c
endif

endif