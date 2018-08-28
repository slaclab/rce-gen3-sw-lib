# File:        constituents.mk
# Abstract:    SD Driver build constituents
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     11/5/2012
# Committed:  \$Date: 2014-05-15 11:25:23 -0700 (Thu, 15 May 2014) $ by \$Author: smaldona $.
# Revision:   \$Revision: 3308 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/driver/sd/constituents.mk $
# Credits:     SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
ifeq ($(have_rtems),$(true))
ifeq ($(have_zynq),$(true))

libnames := Sd

libsrcs_Sd :=

libsrcs_Sd += src/init.c
libsrcs_Sd += src/armCA9/sd_register.c
libsrcs_Sd += src/mmc.c
libsrcs_Sd += src/sdhci.c

#modnames := fatFormat rfsFormat multiBlock rawRate fsRate rateStats printStatus
#modnames += fsWrite fsVerify

majorv_fatFormat  := 1
minorv_fatFormat  := 0
branch_fatFormat  := src
modsrcs_fatFormat := src/fatFormat.cc
modslib_fatFormat := gcc

majorv_rfsFormat  := 1
minorv_rfsFormat  := 0
branch_rfsFormat  := src
modsrcs_rfsFormat := src/rfsFormat.cc
modslib_rfsFormat := gcc

majorv_multiBlock  := 1
minorv_multiBlock  := 0
branch_multiBlock  := test
modsrcs_multiBlock := test/multiBlock.cc
modslib_multiBlock := gcc

majorv_rawRate  := 1
minorv_rawRate  := 0
branch_rawRate  := test
modsrcs_rawRate := test/rawRate.cc
modslib_rawRate := gcc

majorv_fsRate  := 1
minorv_fsRate  := 0
branch_fsRate  := test
modsrcs_fsRate := test/fsRate.cc
modslib_fsRate := gcc

majorv_fsWrite  := 1
minorv_fsWrite  := 0
branch_fsWrite  := test
modsrcs_fsWrite := test/fsWrite.cc
modslib_fsWrite := gcc

majorv_fsVerify  := 1
minorv_fsVerify  := 0
branch_fsVerify  := test
modsrcs_fsVerify := test/fsVerify.cc
modslib_fsVerify := gcc

majorv_rateStats  := 1
minorv_rateStats  := 0
branch_rateStats  := test
modsrcs_rateStats := test/rateStats.cc
modslib_rateStats := gcc

majorv_printStatus  := 1
minorv_printStatus  := 0
branch_printStatus  := test
modsrcs_printStatus := test/printStatus.cc
modslib_printStatus := gcc

endif

#modnames += fatDump2 fatLoad fatFrag

majorv_fatDump2  := 1
minorv_fatDump2  := 0
branch_fatDump2  := test
modsrcs_fatDump2 := test/fatDump2.cc
modsrcs_fatDump2 += test/fatLib.cc
modslib_fatDump2 := gcc

majorv_fatLoad  := 1
minorv_fatLoad  := 0
branch_fatLoad  := test
modsrcs_fatLoad := test/fatLoad.cc
modsrcs_fatLoad += test/fatLib.cc

majorv_fatFrag  := 1
minorv_fatFrag  := 0
branch_fatFrag  := test
modsrcs_fatFrag := test/fatFrag.cc
modsrcs_fatFrag += test/fatLib.cc

endif
