# comments...

libnames  := 
tgtsrcdir := src

libs_common :=
libs_common += tool/csi
libs_common += tool/memory
libs_common += tool/concurrency
libs_common += tool/exception
libs_common += tool/debug
libs_common += tool/kvt
libs_common += tool/hash
libs_common += tool/string
libs_common += tool/io
libs_common += service/udp
libs_common += service/net
libs_common += service/logger
libs_common += service/dsl_l
libs_common += service/atca_l

ifeq ($(or $(have_rce),$(have_cm)),$(true))
libs_common += tool/map tool/mem
endif

ifeq ($(have_linux),$(true))

ifeq ($(have_desktop),$(true))
ifeq ($(have_64bit),$(false))
_syslib_rt := /usr/lib/rt
else
_syslib_rt := /usr/lib64/rt
endif
endif

#command line executables for linux desktop
tgtnames :=
tgtnames += atca_ip
tgtsrcs_atca_ip := $(tgtsrcdir)/atca_ip.cc
tgtlibs_atca_ip := $(libs_common)
tgtslib_atca_ip := $(_syslib_rt)

tgtnames += atca_mac
tgtsrcs_atca_mac := $(tgtsrcdir)/atca_mac.cc
tgtlibs_atca_mac := $(libs_common)
tgtslib_atca_mac := $(_syslib_rt)

ifeq ($(have_rce),$(true))
tgtnames += atca_server
tgtsrcs_atca_server := $(tgtsrcdir)/atca_server.cc
tgtlibs_atca_server := $(libs_common)
tgtslib_atca_server := $(_syslib_rt)
endif

tgtnames += atca_dump
tgtsrcs_atca_dump := $(tgtsrcdir)/atca_dump.cc
tgtlibs_atca_dump := $(libs_common)
tgtslib_atca_dump := $(_syslib_rt)
endif

ifeq ($(have_rtems),$(true))
#RTEMS only test modules
modlibnames := 
modnames :=

#modnames += atca_dump
modsrcs_atca_dump := test/rtems/atca_dump.cc
majorv_atca_dump := 0
minorv_atca_dump := 0
branch_atca_dump := main

#modnames += atca_mac
modsrcs_atca_mac := test/rtems/atca_mac.cc
majorv_atca_mac := 0
minorv_atca_mac := 0
branch_atca_mac := main

#modnames += atca_ip
modsrcs_atca_ip := test/rtems/atca_ip.cc
majorv_atca_ip := 0
minorv_atca_ip := 0
branch_atca_ip := main
endif

ifeq ($(have_linux),$(true))
tgtnames += shelf_ping
tgtsrcs_shelf_ping := $(tgtsrcdir)/shelf_ping.cc
tgtlibs_shelf_ping := $(libs_common)
tgtslib_shelf_ping := $(_syslib_rt)

ifeq ($(have_rce),$(true))
tgtnames += shelf_ping_server
tgtsrcs_shelf_ping_server := $(tgtsrcdir)/shelf_ping_server.cc
tgtlibs_shelf_ping_server := $(libs_common)
tgtslib_shelf_ping_server := $(_syslib_rt) pthread
endif

tgtnames += remote_service
tgtsrcs_remote_service := $(tgtsrcdir)/remote_service.cc
tgtlibs_remote_service := $(libs_common)
tgtslib_remote_service := $(_syslib_rt)

ifeq ($(have_rce),$(true))
tgtnames += remote_server
tgtsrcs_remote_server := $(tgtsrcdir)/remote_server.cc
tgtlibs_remote_server := $(libs_common)
tgtslib_remote_server := $(_syslib_rt)
endif

endif
