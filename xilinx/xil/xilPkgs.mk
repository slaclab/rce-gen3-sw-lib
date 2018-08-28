# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        xilPkgs.mk
# Abstract:    Build a list of Xilinx packages files might be included from
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     9/9/2013
# Committed:  \$Date: 2013-10-31 18:45:51 -0700 (Thu, 31 Oct 2013) $ by \$Author: claus $.
# Revision:   \$Revision: 2251 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/xilinx/xil/xilPkgs.mk $
# Credits:     SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
xilPrj := $(RELEASE_DIR)/xilinx

bram         := bram_v3_02_a
canps        := canps_v1_01_a
common       := common_v1_00_a
cpu_cortexa9 := cpu_cortexa9_v1_01_a
devcfg       := devcfg_v2_03_a
dmaps        := dmaps_v1_05_a
emacps       := emacps_v1_04_a
gpiops 	     := gpiops_v1_01_a
iicps        := iicps_v1_03_a
lwip140      := lwip140_v1_05_a
qspips       := qspips_v2_02_a
scugic       := scugic_v1_04_a
scutimer     := scutimer_v1_02_a
scuwdt       := scuwdt_v1_02_a
standalone   := standalone_v3_10_a
ttcps  	     := ttcps_v1_01_a
uartps       := uartps_v1_04_a
usbps  	     := usbps_v1_04_a
wdtps  	     := wdtps_v1_02_a
xadcps 	     := xadcps_v1_01_a

_xilPkgs :=
_xilPkgs += $(bram)
_xilPkgs += $(canps)
_xilPkgs += $(common)
_xilPkgs += $(cpu_cortexa9)
_xilPkgs += $(devcfg)
_xilPkgs += $(dmaps)
_xilPkgs += $(emacps)
_xilPkgs += $(gpiops)
_xilPkgs += $(iicps)
#_xilPkgs += $(lwip140) # No dependency on this yet
_xilPkgs += $(qspips)
_xilPkgs += $(scugic)
_xilPkgs += $(scutimer)
_xilPkgs += $(scuwdt)
#ifeq ($(have_rtems),$(false))
_xilPkgs += $(standalone)
#endif
_xilPkgs += $(ttcps)
_xilPkgs += $(uartps)
_xilPkgs += $(usbps)
_xilPkgs += $(wdtps)
_xilPkgs += $(xadcps)

xilPkgs := $(addprefix $(xilPrj)/xil/src/, $(addsuffix /src, $(_xilPkgs)))
