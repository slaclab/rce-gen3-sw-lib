# comments...

ifeq ($(have_linux),$(true))
ifeq ($(have_rce),$(true))

libs_common :=
libs_common += tool/csi
libs_common += tool/mem
libs_common += tool/memory
libs_common += tool/concurrency
libs_common += tool/exception
libs_common += tool/debug
libs_common += tool/string
libs_common += tool/io
libs_common += tool/map
libs_common += service/udp
libs_common += service/net
libs_common += service/dsl_l
libs_common += service/atca_l
libs_common += service/dslutil
libs_common += service/logger
libs_common += tool/hash
libs_common += tool/kvt

tgtnames += rced
tgtsrcs_rced := src/rced.cc
tgtlibs_rced := $(libs_common)

tgtnames += sysinfo
tgtsrcs_sysinfo := src/sysinfo.c
tgtlibs_sysinfo := tool/map 
tgtlibs_sysinfo += tool/mem
tgtlibs_sysinfo += tool/debug

endif
endif
