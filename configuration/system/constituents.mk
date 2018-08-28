# -*-Mode: Makefile;-*-
#                               Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Abstract:    constituents.mk for configuration/system
# Author:      R. Claus  ---  claus@slac.stanford.edu
# Created:     2011/04/22
# Commit:     \$Date: 2014-05-15 11:25:23 -0700 (Thu, 15 May 2014) $ by \$Author: smaldona $.
# Revision:   \$Revision: 3308 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/configuration/system/constituents.mk $
# Credits:     SLAC

ifeq ($(tgt_os),rtems)
libnames := system

libsrcs_system :=

libsrcs_system += src/ConfigDb.cc
endif
