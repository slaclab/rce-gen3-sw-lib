# -*-Mode: Makefile;-*-
#
# Facility:     DAT
# File:         constituents.mk
# Abstract:     Constituents file for LwIP test package.
# Author:       R. Claus <claus@slac.stanford.edu>
# Created:      Thu Oct  3 18:51:33 2013
# Committed:  \$Date: 2014-03-11 17:51:32 -0700 (Tue, 11 Mar 2014) $ by \$Author: claus $.
# Revision:   \$Revision: 2912 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/test/lwip/constituents.mk $
# Credits:      SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.

ifeq ($(and $(have_rtems),$(have_zynq)),$(true))

libnames := lwipAdapter

include $(RELEASE_DIR)/xilinx/xil/xilPkgs.mk

PORT := src

COMMON_SRCS = $(PORT)/netif/xpqueue.c \
	      $(PORT)/netif/xadapter.c \
	      $(PORT)/netif/xtopology_g.c

PS7_ETHERNET_SRCS = $(PORT)/netif/xemacpsif_hw.c \
	     $(PORT)/netif/xemacpsif_physpeed.c \
	     $(PORT)/netif/xemacpsif.c		\
	     $(PORT)/netif/xemacpsif_dma.c

ADAPTER_SRCS = $(COMMON_SRCS)

ADAPTER_SRCS += $(PS7_ETHERNET_SRCS)

LWIP_ROOT = $(RTEMS_ROOT)/lwip

CPPFLAGS += -I$(RELEASE_DIR)/configuration/xilinx
CPPFLAGS += $(addprefix -I, $(xilPkgs))
CPPFLAGS += -I$(RTEMS_MAKEFILE_PATH)/lib/include/lwip
CPPFLAGS += -I$(RTEMS_MAKEFILE_PATH)/lib/include/lwip/ipv4
CPPFLAGS += -I$(RTEMS_MAKEFILE_PATH)/lib/include/lwip/ipv6
CPPFLAGS += -Isrc/include
CPPFLAGS += -DCONFIG_LINKSPEED1000

libsrcs_lwipAdapter += src/init.cc $(ADAPTER_SRCS)


tgtnames := netTest loopbackTest

libs :=

libs += \
    tool/exception \
    tool/mem \
    tool/time \
    tool/tdebug \
    xilinx/xil \
    test/lwipAdapter

tgtsLib := \
    $(RTEMS_DIR)/rtemscpu \
    $(RTEMS_DIR)/rtemsbsp \
    $(RTEMS_DIR)/lwip

srcDir := ../../platform/startup/src

bspOverrides := $(tgt_cpu_family)/bspstarthooks.c $(tgt_cpu_family)/zynq-uart.c

tgtsrcs_netTest := test/netTest.cc
tgtsrcs_netTest += $(addprefix $(srcDir)/, $(bspOverrides))
tgtlibs_netTest := $(libs)
tgtslib_netTest := $(tgtsLib)

tgtsrcs_loopbackTest := test/loopbackTest.cc
tgtsrcs_loopbackTest += $(addprefix $(srcDir)/, $(bspOverrides))
tgtlibs_loopbackTest := $(libs)
tgtslib_loopbackTest := $(tgtsLib)

tgtsrcs_iperf_s := test/iperf_s.cc
tgtsrcs_iperf_s += test/iperf.cc
tgtsrcs_iperf_s += $(addprefix $(srcDir)/, $(bspOverrides))
tgtlibs_iperf_s := $(libs) benchmark/iperf benchmark/compat
tgtslib_iperf_s := $(tgtsLib)

endif
