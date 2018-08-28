# -*-Mode: Makefile;-*-
#                               Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Facility:    DAT
# File:        rules.mk
# Abstract:    Special make rules for the Xilinx Memory Test
# Author:      claus <claus@slac.stanford.edu>
# Created:     12/8/2011
# Committed:  \$Date: 2012-11-17 16:47:13 -0800 (Sat, 17 Nov 2012) $ by \$Author: claus $.
# Revision:   \$Revision: 1376 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/bootstrap/memTestX/rules.mk $
# Credits:     SLAC
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

# Build only for the PPC based RCEs without full C (or C++) runtime support.
ifeq ($(and $(have_rce),$(have_ppc)),$(true))
ifeq ($(have_std_c),$(false))

bins := $(bindir)/memTestX.lst

.PHONY: clean_memTestX
clean:  clean_memTestX

# The fully linked code in the ELF format used by the GNU tools.
$(bindir)/memTestX: linkScr.ld

$(bindir)/memTestX: LXFLAGS := -TlinkScr.ld

# A full disassembly of the linked code.
$(bindir)/memTestX.lst: $(bindir)/memTestX
	@echo "[LB] {$(tgt_arch)} Build memTestX.lst"
	$(quiet)$(OBJDUMP) --syms --disassemble-all $< > $@
endif
endif

bin: $(bins)

clean_memTestX:
	@echo "[RB] {$(tgt_arch)} Removing memTestX, .lst, and .map"
	$(quiet)rm -f $(bindir)/memTestX{,.lst,.map}
