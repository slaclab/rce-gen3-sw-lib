# -*-Mode: Makefile;-*-
#
# Facility: 	DAT
# File:     	memTest.mk
# Abstract: 	Special make rules for Virtex4 memTest code that will go into block RAM.
# Author:   	R. Claus <claus@slac.stanford.edu>
# Created:      2011/05/09
# Committed:  \$Date: 2012-11-05 18:26:57 -0800 (Mon, 05 Nov 2012) $ by \$Author: claus $.
# Revision:   \$Revision: 1356 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/bootstrap/memTest/rules.mk $
# Credits:      SLAC
#                              Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.

# The memTest code is run just after processor reset and does not
# include RTEMS or any high-level language support. It does not export
# symbols to other code and so does not need a dynamic section or a
# dynamic symbol table. It will run in Virtex block RAM which is
# assigned to certain special addresses at boot time which will become
# inaccessible once the RTEMS application startup code turns on the
# MMU. All this means that we have to use a custom linker script.

# Build only for the PPC based RCEs
ifeq ($(tgt_cpu_family)-$(tgt_os),ppc-rtems)
_VERSION := 0x00010000

# Append to the standard "bin" target to make the executable file
# after the source files have been compiled into the lib*.a file
# (not a library despite the name, just a merged object file).
bins := $(bindir)/memTest.elf $(bindir)/memTest.lst

.PHONY: clean_memTest
clean:  clean_memTest

# The fully linked code in the ELF format used by the GNU tools
$(bindir)/memTest: $(libdir)/libmemTest.a linkscr.ld
	@echo "[LB] {$(tgt_arch)} Build memTest and .map"
	$(quiet)$(LD) -L$(RELEASE_DIR)/make/sw/$(tgt_board)       \
                      -Tlinkscr.ld                                \
                      --defsym version=$(_VERSION)                \
                      -Map $@.map                                 \
                      -o $@                                       \
                      $(libdir)/libmemTest.a

# For flash use: Strip the executable of extraneous symbols and sections
$(bindir)/memTest.elf: $(bindir)/memTest
	@echo "[LB] {$(tgt_arch)} Build memTest.elf"
	$(quiet)$(STRIP) -p -o $@ $<

# A full disassembly of the linked code.
$(bindir)/memTest.lst: $(bindir)/memTest
	@echo "[LB] {$(tgt_arch)} Build memTest.lst"
	$(quiet)$(OBJDUMP) --syms --disassemble-all $< > $@

ifeq ($(tgt_board),rceG1)
# Append to the standard "bin" target to make the executable file
# after the source files have been compiled into the lib*.a file
# (not a library despite the name, just a merged object file).
bins += $(bindir)/memTest_old.elf $(bindir)/memTest_old.lst

.PHONY: clean_memTest_old
clean:  clean_memTest_old

# The fully linked code in the ELF format used by the GNU tools
$(bindir)/memTest_old: $(libdir)/libmemTest_old.a linkscr.ld
	@echo "[LB] {$(tgt_arch)} Build memTest_old and .map"
	$(quiet)$(LD) -L$(RELEASE_DIR)/make/sw/$(tgt_board)       \
                      -Tlinkscr.ld                                \
                      --defsym version=$(_VERSION)                \
                      -Map $@.map                                 \
                      -o $@                                       \
                      $(libdir)/libmemTest_old.a

# For flash use: Strip the executable of extraneous symbols and sections
$(bindir)/memTest_old.elf: $(bindir)/memTest_old
	@echo "[LB] {$(tgt_arch)} Build memTest_old.elf"
	$(quiet)$(STRIP) -p -o $@ $<

# A full disassembly of the linked code.
$(bindir)/memTest_old.lst: $(bindir)/memTest_old
	@echo "[LB] {$(tgt_arch)} Build memTest_old.lst"
	$(quiet)$(OBJDUMP) --syms --disassemble-all $< > $@

endif
endif

bin: $(bins)

clean_memTest:
	@echo "[RB] {$(tgt_arch)} Removing memTest, .elf, .lst, and .map"
	$(quiet)rm -f $(bindir)/memTest{,.elf,.lst,.map}

clean_memTest_old:
	@echo "[RB] {$(tgt_arch)} Removing memTest_old, .elf, .lst, and .map"
	$(quiet)rm -f $(bindir)/memTest_old{,.elf,.lst,.map}
