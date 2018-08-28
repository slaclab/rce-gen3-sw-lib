# -*-Mode: Makefile;-*-
#
# Facility:     DAT
# File:         rules.mk
# Abstract:     Special make rules for the boot loader code.
# Author:       R. Claus <claus@slac.stanford.edu>
# Created:      10/31/2011
# Committed:  \$Date: 2016-05-17 13:05:25 -0700 (Tue, 17 May 2016) $ by \$Author: smaldona $
# Revision:   \$Revision: 4808 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/bootstrap/bootLoader/rules.mk $
# Credits:      SLAC
#                              Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

bins :=

## Build only for the PPC based RCEs
#ifeq ($(and $(have_rtems),$(have_ppc)),$(true))
#_VERSION := 0x00020001
#
## Append to the standard "bin" target to make the executable file
## after the source files have been compiled into the lib*.a file
## (not a library despite the name, just a merged object file).
#bins += $(bindir)/bootLoader.elf
#
#.PHONY: clean_bootLoader
#clean:  clean_bootLoader
#
## The fully linked code in the ELF format for use with gdb, etc.
#$(bindir)/bootLoader: $(libdir)/libbootLoader.a ../common/bootstrap.ld
#	 @echo "[LB] {$(tgt_arch)} Build bootLoader, .map"
#	 $(quiet)$(LD) -L$(RELEASE_DIR)/make/sw/$(tgt_board)       \
#		       -T../common/bootstrap.ld                    \
#		       --defsym version=$(_VERSION)                \
#		       -Map $@.map                                 \
#		       -o $@                                       \
#		       $(libdir)/libbootLoader.a
#
## For flash use: Strip the executable of extraneous symbols and sections
#$(bindir)/bootLoader.elf: $(bindir)/bootLoader
#	 @echo "[LB] {$(tgt_arch)} Build bootLoader.elf, lst"
#	 $(quiet)$(STRIP) -p -o $@ $<
#	 $(quiet)$(OBJDUMP) --syms --disassemble-all $< > $@
#
#endif


# Build only for the ARM based boards
ifeq ($(and $(have_noose),$(have_zynq)),$(true))

include $(RELEASE_DIR)/xilinx/xil/xilPkgs.mk

# Append to the standard "bin" target to make the executable file
bins += $(bindir)/boot.bin

ubDir  := $(UBOOT_ROOT)
cfgSrc := $(RELEASE_DIR)/configuration

ifeq      ($(have_cm),$(true))
tgt_ub := rce
else ifeq ($(have_rce),$(true))
tgt_ub := rce
else      # One of the evaluation boards (ZC702 or Zedboard)
tgt_ub := zed
endif

.PHONY: clean_bootLoader
clean:  clean_bootLoader

# The following is an ordered list: The FSBL must be first in the list and
# the entry point must be in the second ELF file listed
bifSrcs        := $(bindir)/bootLoader.elf        # Partition 0
uBootBifSrcs   := $(bifSrcs)

ifeq      ($(have_cm),$(true))
uBootBifSrcs   += /afs/slac/g/cci/sdimages/current/dtm/BOOT/fpga.bit   # Partition 1
else ifeq ($(have_rce),$(true))
uBootBifSrcs   += /afs/slac/g/cci/sdimages/current/dpm/BOOT/fpga.bit   # Partition 1
else
uBootBifSrcs   += /afs/slac/g/cci/sdimages/current/$(tgt_ub)/BOOT/fpga.bit   # Partition 1
endif

uBootBifSrcs   += $(ubDir)/u-boot_$(tgt_ub).elf   # Partition 2

# Add to the global CPPFLAGS or dependency rules don't see the change
# Avoid changing the global LSCRIPT and LXFLAGS
#CPPFLAGS += -DFSBL_DEBUG_INFO=DEBUG_GENERAL
#CPPFLAGS += -DFSBL_DEBUG_INFO=DEBUG_INFO
#CPPFLAGS += -DFSBL_PERF=1
CPPFLAGS += -I$(incfarm)/xilinxcfg
CPPFLAGS += $(addprefix -I, $(xilPkgs))
CPPFLAGS += -I$(xilPrj)/fsbl/src

FSBL_LSCRIPT    = $(xilPrj)/fsbl/src/lscript.ld
BL_LXFLAGS      = -fno-builtin -nodefaultlibs -mcpu=$(tgt_cpu) $(tgt_flt)
BL_LXFLAGS     += -Wl,--start-group,-lgcc,-lc,--end-group

# The fully linked code in the ELF format used by the GNU tools.
# _Replace_ the LXFLAGS with our own to get rid of -nostartfiles
$(bindir)/bootLoader: $(FSBL_LSCRIPT)
$(bindir)/bootLoader: LXFLAGS = -Wl,-T$(FSBL_LSCRIPT) $(BL_LXFLAGS) $(DEFINES)
$(bindir)/bootLoader:
	@echo "[LB] {${tgt_arch}} Build bootLoader"
	$(quiet)$(LX) $(tgtobjs_bootLoader) $(LXFLAGS) $(linkflags_bootLoader)  -Wl,-Map,$(bindir)/bootLoader.map -o $@

# For flash use: Strip the executable of extraneous symbols and sections
$(bindir)/bootLoader.elf: $(bindir)/bootLoader
	@echo "[LB] {$(tgt_arch)} Build bootLoader.elf, .lst"
	$(quiet)$(STRIP) -p -o $@ $<
	$(quiet)$(OBJDUMP) --syms --disassemble-all $< > $(basename $@).lst

$(bindir)/boot.bif: $(uBootBifSrcs)
	@echo "[LB] {$(tgt_arch)} Build boot.bif"
	$(quiet)echo -e "image: {\n  [bootloader] $(foreach s,$(uBootBifSrcs),$(s)\n)}" > $@

# Produce the SD card boot image
$(bindir)/boot.bin: $(bindir)/boot.bif
	@echo "[LB] {$(tgt_arch)} Build boot.bin"
	$(quiet)$(BOOTGEN) -image $< -o i $@ -w on

bin: $(bins)


clean_bootLoader:
	@echo "[RB] {$(tgt_arch)} Removing $(bins), bootLoader, .elf, .lst and .map"
	$(quiet)rm -f $(bins) $(bindir)/bootLoader{,.elf,.lst,.map}

endif

# Install only for the PPC based RCEs
ifeq ($(or $(and $(have_rtems),$(have_ppc)),$(and $(have_noose),$(have_zynq))),$(true))

# Installation -- install bootLoader, bootLoader.elf and bootLoader.lst
ifloc := $(strip $(install_bin))
install_bootLoader :=
$(ifloc):
	$(quiet)mkdir -p $(ifloc)
define ifbase_template
$(ifloc)/$(1): $(bindir)/$(1)
		$(INSTALL) $(INSTALLFLAGS) -m 0444 $(bindir)/$(1) $(ifloc)
install_bootLoader += $(ifloc)/$(1)
endef
$(foreach if,$(notdir $(bins)),$(eval $(call ifbase_template,$(if))))

install: $(install_bootLoader)
endif
