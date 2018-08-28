# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        rules.mk
# Abstract:    Special make rules for a generic application.
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     12/12/2011
# Committed:  \$Date: 2012-11-17 16:48:28 -0800 (Sat, 17 Nov 2012) $ by \$Author: claus $.
# Revision:   \$Revision: 1378 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/bootstrap/testMainApp/rules.mk $
# Credits:     SLAC
#                               Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

# Build only for the PPC based RCEs without full C (or C++) runtime support.
ifeq ($(and $(have_rce),$(have_ppc)),$(true))
ifeq ($(have_std_c),$(false))

_VERSION := 0x00010000

bins := $(bindir)/testMainApp.elf $(bindir)/testMainApp.lst

.PHONY: clean_testMainApp
clean:  clean_testMainApp

# The fully linked code in the ELF format used by the GNU tools
$(bindir)/testMainApp: linkScr.ld

$(bindir)/testMainApp: LXFLAGS := -Wl,--defsym,version=$(_VERSION),-TlinkScr.ld

# For flash use: Strip the executable of extraneous symbols and sections
$(bindir)/testMainApp.elf: $(bindir)/testMainApp
	@echo "[LB] {$(tgt_arch)} Build testMainApp.elf"
	$(quiet)$(STRIP) -p -o $@ $<

# A full disassembly of the linked code
$(bindir)/testMainApp.lst: $(bindir)/testMainApp
	@echo "[LB] {$(tgt_arch)} Build testMainApp.lst"
	$(quiet)$(OBJDUMP) -t -d $< > $@
endif
endif

bin: $(bins)

clean_testMainApp:
	@echo "[RB] {$(tgt_arch)} Removing testMainApp, .elf, .lst, and .map"
	$(quiet)rm -f $(bindir)/testMainApp{,.elf,.lst,.map}
