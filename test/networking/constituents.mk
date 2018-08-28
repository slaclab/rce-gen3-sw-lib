# -*-Mode: Makefile;-*-
#
# Facility:     DAT
# File:         constituents.mk
# Abstract:     Constituents file for netTest.
# Author:       R. Claus <claus@slac.stanford.edu>
# Created:      Fri Mar 15 19:53:06 2013
# Committed:  \$Date: 2013-08-14 18:28:21 -0700 (Wed, 14 Aug 2013) $ by \$Author: claus $.
# Revision:   \$Revision: 1929 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/test/networking/constituents.mk $
# Credits:      SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.

ifeq ($(and $(or $(have_rce),$(have_cm)),$(have_rtems)),$(true))
tgtnames := netTest

libs :=

libs += \
    tool/concurrency \
    tool/container \
    tool/conversion \
    tool/exception \
    tool/io \
    tool/memory \
    tool/string \
    tool/time \
    service/shell \
    service/dynalink \
    service/debug \
    service/logger

libs += \
    driver/Network

srcdir := src/

tgtsrcs_netTest := $(addprefix $(srcdir), init.cc)
tgtslibs_netTest := \
    $(RTEMS_DIR)/nfs \
    $(RTEMS_DIR)/rtemscpu \
    $(RTEMS_DIR)/telnetd \
    $(RTEMS_DIR)/rtemsbsp
endif
