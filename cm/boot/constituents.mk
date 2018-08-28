# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the boot package.
# Author:      S. Maldonado <smaldona@slac.stanford.edu>
# Created:     08/04/13
# Committed:  \$$ by \$Author:$.
# Revision:   \$Revision:$
# Repository: \$HeadURL:$
# Credits:     SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

ifeq ($(have_arm),$(true))

libnames        := 
libsrcs_boot    :=

# Until operating system independent delay and timer access routines are available,
# u-boot is the noose.
ifeq ($(have_noose),$(true))
libnames         += boot
libsrcs_boot     += src/uboot/ClusterIpInfo.c 
libsrcs_boot     += src/uboot/FmConfig.c 
libsrcs_boot     += src/uboot/FmConfig.c 
libsrcs_boot     += src/uboot/FmEplHandler.c 
libsrcs_boot     += src/uboot/FmEplRegs.c
libsrcs_boot     += src/uboot/FmFidTableEntry.c 
libsrcs_boot     += src/uboot/FmTahoe_init.c 
libsrcs_boot     += src/uboot/FmEplRegsB.c
libsrcs_boot     += src/uboot/FmIngressFidTableEntry.c 
libsrcs_boot     += src/uboot/FmEgressFidTableEntry.c 
libsrcs_boot     += src/uboot/FmIngressVlanTableEntry.c 
libsrcs_boot     += src/uboot/FmEgressVlanTableEntry.c 
libsrcs_boot     += src/uboot/FmBali_init.c 
libsrcs_boot     += src/uboot/cm.c 
libsrcs_boot     += src/uboot/plx.c

endif

endif
