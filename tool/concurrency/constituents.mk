# -*-Mode: Makefile;-*-
#                               Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Abstract: constituents.mk for tool/concurrency


libnames  :=
tgtnames  :=
tasknames :=


##### lib concurrency #####
libsrcs_concurrency :=

ifeq ($(or $(have_rtems),$(have_linux)),$(true))
libnames += concurrency
libsrcs_concurrency += src/SemaphoreGuard.cc
libsrcs_concurrency += src/Thread.cc
endif

ifeq ($(have_linux),$(true))
libsrcs_concurrency += src/linux/Notepad.cc
libsrcs_concurrency += src/linux/Semaphore.cc
libsrcs_concurrency += src/linux/Sleep.cc
libsrcs_concurrency += src/linux/Thread.cc

libslib_concurrency := $(PTH_LIBSLIB)
endif

ifeq ($(have_rtems),$(true))
libsrcs_concurrency += src/rtems/Notepad.cc
libsrcs_concurrency += src/rtems/Semaphore.cc
libsrcs_concurrency += src/rtems/Thread.cc
endif



##### Unit test testConcurrency #####

testSources :=
testSources += test/Philosopher.cc
testSources += test/testConcurrency.cc
testSources += test/testDining.cc
testSources += test/testExceptions.cc
testSources += test/testHello.cc
testSources += test/testSafety.cc
testSources += test/testBasicMutex.cc

ifeq ($(have_linux),$(true))
LXFLAGS += -lpthread
tgtnames += testConcurrency
tgtsrcs_testConcurrency := test/linux/testMain.cc
tgtsrcs_testConcurrency += $(testSources)

tgtlibs_testConcurrency += tool/concurrency
tgtlibs_testConcurrency += service/logger
tgtlibs_testConcurrency += tool/exception
tgtlibs_testConcurrency += tool/string
tgtlibs_testConcurrency += tool/io
endif

ifeq ($(have_rtems),$(true))
tasknames += testConcurrency
tasksrcs_testConcurrency := test/rtems/testMain.cc
tasksrcs_testConcurrency += $(testSources)
basetgt_testConcurrency  := platform/rtems
basemod_testConcurrency  := platform/rts
endif



##### Package docs for end users #####
docsrcs_concurrency    :=
nodocs_concurrency     :=

# Exclude those files that dependency checking will find but which
# should contribute only to the developer docs.
ifeq ($(have_linux),$(true))
nodocs_concurrency  += \
$(wildcard impl/*.h impl/*.hh linux/*.h linux/*.hh src/linux/*.cc src/linux/*.c src/linux/*.hh src/linux/*.h)
endif

ifeq ($(have_rtems),$(true))
nodocs_concurrency  += \
$(wildcard impl/*.h impl/*.hh rtems/*.h rtems/*.hh src/rtems/*.cc src/rtems/*.c src/rtems/*.hh src/rtems/*.h)
endif

# Include selected files that aren't compiled but which contain
# documentation.
ifeq ($(or $(have_rtems),$(have_linux)),$(true))
# Files containing user-level documentation only; no code.
docsrcs_concurrency += src/criticalSection.cc
docsrcs_concurrency += src/Atomic.cc
docsrcs_concurrency += src/BasicMutex.c
docsrcs_concurrency += src/Notepad.cc
docsrcs_concurrency += src/Once.cc
docsrcs_concurrency += src/OnceFlag.c
docsrcs_concurrency += src/Semaphore.cc
docsrcs_concurrency += src/Sleep.cc

# Public header files not referred to by compiled library sources.
docsrcs_concurrency += include/criticalSection.hh
docsrcs_concurrency += include/OnceFlag.h
docsrcs_concurrency += include/Atomic.hh
docsrcs_concurrency += include/BasicMutex.h
docsrcs_concurrency += include/Sleep.hh
endif


##### Package docs for developers #####
devdocsrcs_concurrency :=
nodevdocs_concurrency  :=

# Developer doc files include all those for user docs plus the ones we
# excluded from user docs.
devdocsrcs_concurrency := $(docsrcs_concurrency) $(nodocs_concurrency)

# Headers used in test code.
ifeq ($(or $(have_rtems),$(have_linux)),$(true))
devdocsrcs_concurrency  += test/Philosopher.hh
endif

# Document test code.
ifeq ($(have_linux),$(true))
devdocsrcs_concurrency  += $(tgtsrcs_testConcurrency)
endif

ifeq ($(have_rtems),$(true))
devdocsrcs_concurrency  += $(tasksrcs_testConcurrency)
endif
