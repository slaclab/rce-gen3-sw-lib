# -*-Mode: Makefile;-*-
#                               Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Abstract: constituents.mk for service/logger
# Author: Jim Panetta  ---  panetta@slac.stanford.edu
# Date created: 2011/04/20
# Last commit: \$Date: 2014-12-12 10:27:50 -0800 (Fri, 12 Dec 2014) $ by \$Author: panetta $.
# Revision number: \$Revision: 3852 $
# Location in repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/service/logger/constituents.mk $
# Credits: SLAC

# This packages requires OS support.
ifeq ($(have_noose),$(false))

libnames := logger
libsrcs_logger :=

libsrcs_logger += src/LogMessage.cc
libsrcs_logger += src/Logger.cc
libsrcs_logger += src/LoggerImpl.cc
libsrcs_logger += src/StderrLogger.cc
libsrcs_logger += src/SyslogLogger.cc
libsrcs_logger += test/testLogging.cc

ifeq ($(have_linux),$(true))
libsrcs_logger += src/linux/StderrLogger.cc

tgtnames := testLogger
tgtsrcs_testLogger := test/linux/testMain.cc
tgtsrcs_testLogger += test/testLogging.cc

tgtlibs_testLogger := service/logger
tgtlibs_testLogger += tool/exception
tgtlibs_testLogger += tool/string
tgtlibs_testLogger += tool/concurrency
tgtlibs_testLogger += tool/io
endif

ifeq ($(have_rtems),$(true))
libsrcs_logger += src/rtems/StderrLogger.cc

# Private-to-module library. Useful for testing
# code changes w/o having to create a new system image.
modlibnames := logger_m
libsrcs_logger_m := $(libsrcs_logger)
endif

endif
