# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        rules.mk
# Abstract:    Special make rules for a generic RTEMS application.
# Author:      R. Claus <claus@slac.stanford.edu>
# Created:     12/12/2011
# Committed:  \$Date: 2012-11-05 18:26:57 -0800 (Mon, 05 Nov 2012) $ by \$Author: claus $.
# Revision:   \$Revision: 1356 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/bootstrap/testRtemsApp/rules.mk $
# Credits:     SLAC
#                               Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

# Build only for the RCEs with PowerPCs using the RTEMS compiler
ifeq ($(and $(have_rce),$(have_rtems),$(have_ppc)),$(true))

bins := $(bindir)/testRtemsApp.elf $(bindir)/testRtemsApp.lst
bins += $(bindir)/hello.elf $(bindir)/hello.lst

.PHONY: clean_testRtemsApp clean_hello
clean:  clean_testRtemsApp clean_hello


# For flash use: Strip the executable of extraneous symbols and sections
$(bindir)/testRtemsApp.elf: $(bindir)/testRtemsApp
	@echo "[LB] {$(tgt_arch)} Build testRtemsApp.elf"
	$(quiet)$(STRIP) -p -o $@ $<

# A full disassembly of the linked code
$(bindir)/testRtemsApp.lst: $(bindir)/testRtemsApp
	@echo "[LB] {$(tgt_arch)} Build testRtemsApp.lst"
	$(quiet)$(OBJDUMP) --syms --disassemble-all $< > $@

# For flash use: Strip the executable of extraneous symbols and sections
$(bindir)/hello.elf: $(bindir)/hello
	@echo "[LB] {$(tgt_arch)} Build hello.elf"
	$(quiet)$(STRIP) -p -o $@ $<

# A full disassembly of the linked code
$(bindir)/hello.lst: $(bindir)/hello
	@echo "[LB] {$(tgt_arch)} Build hello.lst"
	$(quiet)$(OBJDUMP) --syms --disassemble-all $< > $@
endif

bin: $(bins)

clean_testRtemsApp:
	@echo "[RB] {$(tgt_arch)} Removing testRtemsApp, .elf, .lst, and .map"
	$(quiet)rm -f $(bindir)/testRtemsApp{,.elf,.lst,.map}

clean_hello:
	@echo "[RB] {$(tgt_arch)} Removing hello, .elf, .lst, and .map"
	$(quiet)rm -f $(bindir)/hello{,.elf,.lst,.map}
