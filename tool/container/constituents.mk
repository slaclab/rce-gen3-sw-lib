# -*-Mode: Makefile;-*-
#                               Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Abstract: constituents.mk for tool/container
# Author: Jim Panetta  ---  panetta@slac.stanford.edu
# Date created: 2011/04/20
# Last commit: \$Date: 2014-12-11 14:19:55 -0800 (Thu, 11 Dec 2014) $ by \$Author: panetta $.
# Revision number: \$Revision: 3844 $
# Location in repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/container/constituents.mk $
# Credits: SLAC



#libnames           := container              
#libsrcs_container  := 
#libsrcs_container  += src/Hash.cc
#libsrcs_container  += src/HashList.cc
#libsrcs_container  += src/HashTable.cc
#
#ifeq ($(have_rtems),$(true))
#
#libs_common += tool/concurrency
#modlibnames := container
#modlibs_container := libs_common
#endif

docsrcs_nolibrary := include/Flink.hh
docsrcs_nolibrary += include/Link.hh
docsrcs_nolibrary += include/LinkedList.hh
docsrcs_nolibrary += include/List.hh
docsrcs_nolibrary += include/Queue.hh
docsrcs_nolibrary += include/Stack.hh
