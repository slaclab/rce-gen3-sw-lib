# -*-Mode: Makefile;-*-
#                               Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Abstract: constituents.mk for tool/memory
# Author: Jim Panetta  ---  panetta@slac.stanford.edu
# Date created: 2011/04/20
# Last commit: \$Date: 2015-01-14 15:42:48 -0800 (Wed, 14 Jan 2015) $ by \$Author: tether $.
# Revision number: \$Revision: 3932 $
# Location in repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/memory/constituents.mk $
# Credits: SLAC

libnames := 
libsrcs_memory :=
libsrcs_mem    :=
libsrcs_umem   :=

tgtnames  :=

tasknames :=

modnames  :=


# The high-level "memory" library (C++ code). Requires OS support.
ifeq ($(have_noose),$(false))
libnames += memory
libsrcs_memory += src/Arena.cc
libsrcs_memory += src/FreeList.cc
endif


# The low-level "mem" library (C code).
ifeq ($(have_arm),$(true))
libnames += mem
libsrcs_mem += src/armCA9/memCache.c
libsrcs_mem += src/armCA9/memFlagsTable.c
libsrcs_mem += src/armCA9/memMmu.c
libsrcs_mem += src/armCA9/memRegion.c
libsrcs_mem += src/armCA9/memResource.c
libsrcs_mem += src/armCA9/memResourceHelpers.S

ifeq ($(have_address_translation),$(false))
#     A default implementation of the mapping of device registers
#     when we can assume that virtual address == real address.
libsrcs_mem += src/armCA9/memMapDeviceRegisters.c
endif
ifeq ($(have_linux),$(true))
libsrcs_mem += src/armCA9-linux/memMapDeviceRegisters.c
endif
ifeq ($(have_rtems),$(true))
libsrcs_mem += src/armCA9-rtems/mem.c
endif
endif

# The "umem" library, a subset of "mem" for U-Boot.
ifeq ($(have_arm),$(true))
ifeq ($(have_noose),$(true))
libnames += umem
libsrcs_umem += src/armCA9/memRegion.c
endif
endif

# Unit test code sources.
testSources :=
testSources += test/armCA9/testProbe.c
testSources += test/armCA9/testFlags.c
testSources += test/armCA9/testFlush.c
testSources += test/armCA9/utility.c


# Unit test program for "mem".
ifeq ($(have_arm),$(true))
ifeq ($(have_linux),$(true))
#tgtnames += testMem
#tgtsrcs_testMem := test/armCA9-linux/testMem_main.c
#tgtsrcs_testMem += $(testSources)
#tgtlibs_testMem := tool/debug
#tgtlibs_testMem += tool/mem
endif
ifeq ($(have_rtems),$(true))
#tasknames += testMem
#tasksrcs_testMem  := test/armCA9-rtems/testMem_module.c
#tasksrcs_testMem  += $(testSources)
#basetgt_testMem   := startup/rtems
endif
endif


# Print memory property maps under RTEMS.
ifeq ($(have_arm),$(true))
ifeq ($(have_rtems),$(true))
tasknames += memscan
tasksrcs_memscan  := test/armCA9-rtems/memscan.c
basetgt_memscan   := startup/rtems

tasknames += regions
tasksrcs_regions := test/armCA9-rtems/regions.c
basetgt_regions  := startup/rtems
endif
endif


# Test of stack overflow detection under RTEMS.
ifeq ($(have_arm),$(true))
ifeq ($(have_rtems),$(true))
tasknames += oroborous
tasksrcs_oroborous := test/armCA9-rtems/oroborous.c
basetgt_oroborous  := platform/rtems
endif
endif


# ARM Linux test of mapping OCM via /dev/ocm instead of /dev/mem.
# Requires that the ocm kernel module be loaded.
ifeq ($(and $(have_arm),$(have_linux)),$(true))
tgtnames += testOcm
tgtsrcs_testOcm := test/armCA9-linux/testOcm.c
endif


# ARM Linux test of obtaining DMA buffers via /dev/dmabuf
# Requires that the dmabuf kernel module be loaded.
ifeq ($(and $(have_arm),$(have_linux)),$(true))
tgtnames += testDmabuf
tgtsrcs_testDmabuf := test/armCA9-linux/testDmabuf.c
endif


# Test of atomic resource allocation under RTEMS. Give the test
# program its own copies of the code in memResource and
# memResourceHelper so that it doesn't use or affect the resource
# calls in rtems.so. It will however allocate some space
# in a Region.
ifeq ($(have_arm),$(true))
ifeq ($(have_rtems),$(true))
tasknames += testMemRes
tasksrcs_testMemRes := test/armCA9-rtems/testMemRes.c src/armCA9/memResourceHelpers.S src/armCA9/memResource.c
basetgt_testMemRes  := platform/rtems
endif
endif




##### Package docs for end users #####

# At present all the C++ code in library "memory" is either obsolete
# or developer-only. The C code in library "mem" is nearly all
# documented in src/mem.c and not in any of the source code files.
# The linux kernel modules are documented in the module source files.

docsrcs_memory    :=
nodocs_memory     :=

# Exclude those files that dependency checking may find but which
# should contribute only to the developer docs.
nodocs_memory  += \
$(wildcard impl/*.h impl/*.hh arm*/*.h arm*/*.hh i86*/*.hh i86*/*.h src/arm*/*.cc src/arm*/*.c src/arm*/*.hh src/arm*/*.h  src/i86*/*.c src/i86*/*.hh src/i86*/*.h)

# Suppress other developer-only information.
nodocs_memory += include/Cache.hh include/FreeList.hh include/PoolEntry.hh src/FreeList.cc include/Arena.hh src/Arena.cc impl/Arena-inl.hh $(libsrcs_memory) $(libsrcs_mem) $(libsrcs_umem)

ifeq ($(have_arm),$(true))
docsrcs_memory += package-arm.doc

# The generic ARM API declarations. The actual header files that users
# include is include/mem.h or resources.h which just include the platform-specific
# headers. At present the only public stuff is in armCA9/.
docsrcs_memory += armCA9/mem.h armCA9/resources.h
nodocs_memory += include/mem.h include/resources.h
docsrcs_memory += src/armCA9/memResource.c

# Include selected files that aren't compiled but which contain
# documentation.
# Files containing user-level documentation only; no code.
docsrcs_memory += src/mem.c

# Public header files not referred to by compiled library sources.
# None.

# Kernel modules for ArchLinux on ARM.
# These aren't compiled into the package libraries so need to be explicitly included and removed
# from the dodocs list.
docsrcs_memory += armCA9-linux/dmabuf.h armCA9-linux/dmabufapi.h src/armCA9-linux/dmabuf.c src/armCA9-linux/ocm.c

# Anything we explicitly list in docsrcs must be removed from nodocs.
nodocs_memory := $(filter-out $(docsrcs_memory),$(nodocs_memory))

else
docsrcs_memory += package-non-arm.doc
endif



##### Package docs for developers #####
devdocsrcs_memory :=
nodevdocs_memory  :=

ifeq ($(have_arm),$(true))
# Developer doc files include all those for user docs plus the ones we
# excluded from user docs.
#devdocsrcs_memory := $(docsrcs_memory) $(nodocs_memory)

# Headers used in test code.
# TBD

# Document test code.
# TBD
endif
