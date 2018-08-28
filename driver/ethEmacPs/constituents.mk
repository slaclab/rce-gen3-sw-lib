# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Ethernet EmacPS build constituents
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     8/13/2013
# Committed:  \$Date: 2014-11-06 13:03:24 -0800 (Thu, 06 Nov 2014) $ by \$Author: smaldona $.
# Revision:   \$Revision: 3820 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/driver/ethEmacPs/constituents.mk $
# Credits:     SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
ifeq ($(and $(have_rtems),$(have_zynq)),$(true))

modnames := ethEmacPs
modsrcs_ethEmacPs += src/ethEmacPs.c
modsrcs_ethEmacPs += src/ethEmacPs_dma.c
modsrcs_ethEmacPs += src/ethEmacPs_hw.c
modsrcs_ethEmacPs += src/ethEmacPs_phySpeed.c
modsrcs_ethEmacPs += src/ethernet.c
basetgt_ethEmacPs := platform/rtems
basemod_ethEmacPs := driver/network

endif
