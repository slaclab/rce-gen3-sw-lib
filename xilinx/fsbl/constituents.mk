# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the Xilinx First Stage Boot Loader.
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     2/13/2013
# Commited:   \$Date: 2014-11-05 12:18:04 -0800 (Wed, 05 Nov 2014) $ by \$Author: panetta $.
# Revision:   \$Revision: 3814 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/xilinx/fsbl/constituents.mk $
# Credits:     SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

do_not_document = $(true)

# Build only for the ZYNQ without full C (or C++) runtime support.
ifeq ($(and $(have_zynq),$(have_noose)),$(true))
libnames := fsbl

libsrcs_fsbl :=

libsrcs_fsbl += src/ddr_init.c
libsrcs_fsbl += src/ff.c
libsrcs_fsbl += src/fsbl_handoff.S
libsrcs_fsbl += src/image_mover.c
libsrcs_fsbl += src/main.c
libsrcs_fsbl += src/md5.c
libsrcs_fsbl += src/mmc.c
libsrcs_fsbl += src/nand.c
libsrcs_fsbl += src/nor.c
libsrcs_fsbl += src/pcap.c
libsrcs_fsbl += src/qspi.c
libsrcs_fsbl += src/sd.c

# Keep these out of the library as they are to be supplied by the tgt program
#libsrcs_fsbl += src/fsbl_hooks.c
#libsrcs_fsbl += src/ps7_init.c

endif
