# -*-Mode: Makefile;-*-
#                               Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Abstract: constituents.mk for tool/conversion
# Author: Jim Panetta  ---  panetta@slac.stanford.edu
# Date created: 2011/04/20
# Last commit: \$Date: 2012-05-04 23:14:32 -0700 (Fri, 04 May 2012) $ by \$Author: panetta $.
# Revision number: \$Revision: 943 $
# Location in repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/conversion/constituents.mk $
# Credits: SLAC

libnames := conversion

libsrcs_conversion := src/BSWP.cc

ifeq ($(tgt_os),rtems)

modlibnames := conversion_m
libsrcs_conversion_m := src/BSWP.cc

endif

