# -*-Mode: Makefile;-*-
#                               Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Abstract: constituents.mk for tool/string
# Author: Jim Panetta  ---  panetta@slac.stanford.edu
# Date created: 2011/04/20
# Last commit: \$Date: 2012-04-11 17:28:30 -0700 (Wed, 11 Apr 2012) $ by \$Author: claus $.
# Revision number: \$Revision: 843 $
# Location in repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/string/constituents.mk $
# Credits: SLAC

libnames := string

libsrcs_string := src/Name.cc
libsrcs_string += src/StringBuffer.cc
libsrcs_string += src/mac2colon.cc
libsrcs_string += src/ip2dot.cc
libsrcs_string += src/id2node.cc
