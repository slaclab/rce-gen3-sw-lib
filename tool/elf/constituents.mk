# -*-Mode: Makefile;-*-
#                               Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Abstract: constituents.mk for tool/elf
# Author: Stephen Tether  ---  tether@slac.stanford.edu
# Date created: 2013/08/21
# Last commit: \$Date: 2015-01-12 10:57:01 -0800 (Mon, 12 Jan 2015) $ by \$Author: tether $.
# Revision number: \$Revision: 3929 $
# Location in repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/elf/constituents.mk $
# Credits: SLAC


# This package provides ELF-handling tools which for now assume that
# no swapping is needed. Thus they can handle 32-bit ARM code on both
# ARM and Intel platforms.

ifeq ($(or $(have_linux),$(have_rtems)),$(true))

libnames := elf
libsrcs_elf :=

modnames :=

tasknames :=

tgtnames :=

# Build a test dynamic object as an RTEMS module since modules satisfy
# the layout constraints listed in test/linux/testElf_main.c. The test
# program itself is built as a Linux program. To run testElf from
# its build directory:
#     ./testElf ../../mod/arm-rtems-rceCA9-xxx/testSubject.1.0.main.so

ifeq ($(have_linux),$(true))
ifeq ($(have_64bit),$(false))
libsrcs_elf += src/linker.c
libsrcs_elf += src/lnkObjDb.c
libsrcs_elf += src/lnkSearchDag.c
libsrcs_elf += src/exidx.c
libsrcs_elf += src/linux/exidxTable.cc

tgtnames += testElf
tgtsrcs_testElf := test/linux/testElf_main.c test/linux/loadElf.c

libnames += testSubject
libsrcs_testSubject := test/linux/testSubject.c

tgtnames += testLinker
tgtsrcs_testLinker := test/linux/testLinker_main.c test/linux/loadElf.c
tgtlibs_testLinker :=
tgtlibs_testLinker += service/logger
tgtlibs_testLinker += tool/concurrency
tgtlibs_testLinker += tool/debug
tgtlibs_testLinker += tool/elf
tgtlibs_testLinker += tool/exception
tgtlibs_testLinker += tool/io
#tgtlibs_testLinker += tool/mem
tgtlibs_testLinker += tool/string
endif
endif

ifeq ($(have_arm),$(true))
ifeq ($(have_rtems),$(true))
libsrcs_elf += src/linker.c
libsrcs_elf += src/lnkObjDb.c
libsrcs_elf += src/lnkSearchDag.c
libsrcs_elf += src/relocator.c
libsrcs_elf += src/exidx.c
libsrcs_elf += src/rtems/exidxTable.c

tasknames += testLookup
tasksrcs_testLookup := test/armCA9-rtems/testLookup.c
basetgt_testLookup  := platform/rtems

endif
endif

endif


##### User-level documentation
docsrcs_elf :=
nodocs_elf :=

docsrcs_elf += include/linker.h include/lnkStatus.h src/linker.c

# Suppress everything except what we know we want.
nodocs_elf += $(wildcard impl/*.* include/*.* src/*.* src/*/*.* test/*.* test/*/*.*)
nodocs_elf := $(filter-out $(docsrcs_elf),$(nodocs_elf))



##### Developer's documentation
# TBD
