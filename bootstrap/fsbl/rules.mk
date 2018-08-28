# -*-Mode: Makefile;-*-
#
# Facility:     DAT
# File:         rules.mk
# Abstract:     Special make rules for the Zynq first stage boot loader code.
# Author:       R. Claus <claus@slac.stanford.edu>
# Created:      4/23/2013
# Committed:  \$Date: 2013-09-20 07:02:17 -0700 (Fri, 20 Sep 2013) $ by \$Author: smaldona $
# Revision:   \$Revision: 2144 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/bootstrap/fsbl/rules.mk $
# Credits:      SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

# Build only for the ARM based boards
ifeq ($(and $(have_zynq),$(have_noose)),$(true))

include $(RELEASE_DIR)/xilinx/xil/xilPkgs.mk

# Append to the standard "bin" target to make the executable file
bins := $(bindir)/boot_memTest.bin

# Build the bin only for the eval boards
ifeq ($(or $(have_rce),$(have_cm)), $(false))
bins += $(bindir)/bist.elf
endif

fwDir   := /afs/slac/g/cci/firmware
cfgDir  := $(RELEASE_DIR)/configuration

ifeq      ($(have_cm),$(true))
fwSrc  := COB/DTM/ZynqDtm
fwVer  := DC000001
else ifeq ($(have_rce),$(true))
fwSrc  := COB/DPM/ZynqDpm
fwVer  := DD000000
else      # One of the evaluation boards (ZC702 or Zedboard)
fwSrc  := eval/ZynqTestNew
fwVer  := F1000102
endif

.PHONY: clean_fsbl clean_bist clean_memTest
clean:  clean_fsbl clean_bist clean_memTest

# Add to the global CPPFLAGS or dependency rules don't see the change
# Avoid changing the global LSCRIPT and LXFLAGS
CPPFLAGS += -DFSBL_DEBUG_INFO
CPPFLAGS += -I$(cfgDir)/xilinx
CPPFLAGS += -I$(xilPrj)/bist/src
CPPFLAGS += $(addprefix -I, $(xilPkgs))
CPPFLAGS += -I$(xilPrj)/fsbl/src

FSBL_LSCRIPT  = src/lscript.ld
FSBL_LXFLAGS  = -fno-builtin -nodefaultlibs -mcpu=$(tgt_cpu) $(tgt_flt)
FSBL_LXFLAGS += -Wl,--start-group,-lgcc,-lc,--end-group

BIST_LSCRIPT = $(xilPrj)/bist/src/lscript.ld
BIST_LXFLAGS = -Wl,--start-group,-lgcc,-lc,--end-group

# FSBL:
# The fully linked code in the ELF format used by the GNU tools.
# _Replace_ the LXFLAGS with our own to get rid of -nostartfiles
$(bindir)/fsbl: $(FSBL_LSCRIPT)
$(bindir)/fsbl: LXFLAGS = -Wl,-T$(FSBL_LSCRIPT) $(FSBL_LXFLAGS)

# For flash use: Strip the executable of extraneous symbols and sections
$(bindir)/fsbl.elf: $(bindir)/fsbl
	@echo "[LB] {$(tgt_arch)} Build fsbl.elf, .lst"
	$(quiet)$(STRIP) -p -o $@ $<
	$(quiet)$(OBJDUMP) --syms --disassemble-all $< > $(basename $@).lst

# bist:
# The fully linked code in the ELF format used by the GNU tools.
$(bindir)/bist: $(BIST_LSCRIPT)
$(bindir)/bist: LXFLAGS := -Wl,-T$(BIST_LSCRIPT) $(BIST_LXFLAGS)

# A full disassembly of the linked code.
$(bindir)/bist.elf: $(bindir)/bist
	@echo "[LB] {$(tgt_arch)} Build bist.elf, .lst"
	$(quiet)$(STRIP) -p -o $@ $<
	$(quiet)$(OBJDUMP) --syms --disassemble-all $< > $(basename $@).lst
	$(quiet)$(SIZE) $<

# memTest:
$(bindir)/memTest: $(FSBL_LSCRIPT)
$(bindir)/memTest: LXFLAGS = -Wl,-T$(FSBL_LSCRIPT) $(FSBL_LXFLAGS)

$(bindir)/memTest.elf: $(bindir)/memTest
	@echo "[LB] {$(tgt_arch)} Build memTest.elf, .lst"
	$(quiet)$(STRIP) -p -o $@ $<
	$(quiet)$(OBJDUMP) --syms --disassemble-all $< > $(basename $@).lst
	$(quiet)$(SIZE) $<

$(bindir)/boot_memTest.bif: $(bindir)/memTest.elf
	@echo "[LB] {$(tgt_arch)} Build boot_memTest.bif"
	$(quiet)echo -e "image: {\n  [bootloader] $<\n}" > $@

# Produce the SD card boot image
$(bindir)/boot_memTest.bin: $(bindir)/boot_memTest.bif
	@echo "[LB] {$(tgt_arch)} Build boot_memTest.bin"
	$(quiet)$(BOOTGEN) -image $< -o i $@ -w on

endif

bin: $(bins)

clean_fsbl:
	@echo "[RB] {$(tgt_arch)} Removing fsbl, .elf, .lst and .map"
	$(quiet)rm -f $(bindir)/fsbl{,.elf,.lst,.map}

clean_bist:
	@echo "[RB] {$(tgt_arch)} Removing bist.elf, .lst, and .map"
	$(quiet)rm -f $(bindir)/bist{.elf,.lst,.map}

clean_memTest:
	@echo "[RB] {$(tgt_arch)} Removing memTest, .elf, .lst and .map"
	$(quiet)rm -f $(bindir)/memTest{,.elf,.lst,.map}


# Install only for the ARM based boards
ifeq ($(and $(have_noose),$(have_zynq)),$(true))

# Installation -- install fsbl, fsbl.elf and fsbl.lst
ifloc := $(strip $(install_bin))
install_fsbl :=
$(ifloc):
	$(quiet)mkdir -p $(ifloc)
define ifbase_template
$(ifloc)/$(1): $(bindir)/$(1)
		$(INSTALL) $(INSTALLFLAGS) -m 0444 $(bindir)/$(1) $(ifloc)
install_fsbl += $(ifloc)/$(1)
endef
$(foreach if,$(notdir $(bins)),$(eval $(call ifbase_template,$(if))))

install: $(install_fsbl)
endif
