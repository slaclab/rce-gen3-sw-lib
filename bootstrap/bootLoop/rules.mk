# -*-Mode: Makefile;-*-
#
# Facility:  	DAT
# File:         rules.mk
# Abstract:  	Special make rules for bootLoop code that will go into block RAM.
# Author:    	R. Claus <claus@slac.stanford.edu>
# Created:      10/27/2011
# Committed:  \$Date: 2012-11-05 18:26:57 -0800 (Mon, 05 Nov 2012) $ by \$Author: claus $.
# Revision:   \$Revision: 1356 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/bootstrap/bootLoop/rules.mk $
# Credits:      SLAC
#                              Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

# Build only for the RCEs
ifeq ($(and $(have_rtems),$(have_ppc)),$(true))
_VERSION := 0x00010000

# Append to the standard "bin" target to make the executable file
# after the source files have been compiled into the lib*.a file
# (not a library despite the name, just a merged object file).
bins := $(bindir)/bootLoop.elf

.PHONY: clean_bootLoop
clean:  clean_bootLoop

# The fully linked code in the ELF format used by the GNU tools.
$(bindir)/bootLoop: $(libdir)/libbootLoop.a ../common/bootstrap.ld
	@echo "[LB] {$(tgt_arch)} Build bootLoop and .map"
	$(quiet)$(LD) -L$(RELEASE_DIR)/make/sw/$(tgt_board)       \
                      -T../common/bootstrap.ld                    \
                      --defsym version=$(_VERSION)                \
                      -Map $(bindir)/bootLoop.map                 \
                      -o $@                                       \
                      $(libdir)/libbootLoop.a

# For flash use: Strip the executable of extraneous symbols and sections
$(bindir)/bootLoop.elf: $(bindir)/bootLoop
	@echo "[LB] {$(tgt_arch)} Build bootLoop.elf"
	$(quiet)$(STRIP) -p -o $@ $<
endif

bin: $(bins)

clean_bootLoop:
	@echo "[RB] {$(tgt_arch)} Removing bootLoop, .elf and .map"
	$(quiet)rm -f $(bindir)/bootLoop{,.elf,.map}
