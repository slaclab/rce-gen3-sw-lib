# -*-Mode: Makefile;-*-
#                               Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Abstract: constituents.mk for tool/concurrency
# Author: Jim Panetta  ---  panetta@slac.stanford.edu
# Date created: 2011/04/20
# Last commit: \$Date: 2014-02-12 13:34:48 -0800 (Wed, 12 Feb 2014) $ by \$Author: panetta $.
# Revision number: \$Revision: 2767 $
# Location in repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/dataStructure/constituents.mk $
# Credits: SLAC

testSources := $(foreach name, Main DataStructure BitRota BitSet, test/test$(name).cc)

# So far all the user library code is in .hh files so we don't build a
# library.
ifeq ($(have_linux),$(true))

testLibs :=
testLibs += tool/concurrency
testLibs += tool/exception
testLibs += tool/io
testLibs += tool/string
testLibs += service/logger

tgtnames := testDataStructures

tgtsrcs_testDataStructures := $(testSources)
tgtlibs_testDataStructures := $(testLibs)

tgtslib_testDataStructures :=
ifeq ($(have_desktop),$(true))
tgtslib_testDataStructures += /usr/lib/pthread
tgtslib_testDataStructures += /usr/lib/rt
endif
endif

ifeq ($(have_rtems),$(true))
# RTEMS
#modnames := testDataStructures
majorv_testDataStructures := 1
minorv_testDataStructures := 0
branch_testDataStructures := main
modsrcs_testDataStructures := $(testSources)
endif
