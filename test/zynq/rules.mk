# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        rules.mk
# Abstract:    Special make rules for the Zynq basic test
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     2/5/2013
# Committed:  \$Date: 2013-08-14 18:28:21 -0700 (Wed, 14 Aug 2013) $ by \$Author: claus $.
# Revision:   \$Revision: 1929 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/test/zynq/rules.mk $
# Credits:     SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

# Build only for the PPC based RCEs without full C (or C++) runtime support.
ifeq ($(have_zynq),$(true))
ifeq ($(have_std_c),$(false))

bins := $(bindir)/test.lst

.PHONY: clean_test
clean:  clean_test

# The fully linked code in the ELF format used by the GNU tools.
$(bindir)/test: linkScr.ld

$(bindir)/test: LXFLAGS := -TlinkScr.ld

# A full disassembly of the linked code.
$(bindir)/test.lst: $(bindir)/test
	@echo "[LB] {$(tgt_arch)} Build test.lst"
	$(quiet)$(OBJDUMP) --disassemble-all $< > $@

bin: $(bins)

clean_test:
	@echo "[RB] {$(tgt_arch)} Removing test, .lst, and .map"
	$(quiet)rm -f $(bindir)/test{,.lst,.map}
endif
endif
