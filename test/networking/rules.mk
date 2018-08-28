# -*-Mode: Makefile;-*-
#
# Facility:     DAT
# File:         rules.mk
# Abstract:     Special make rules.
# Author:       R. Claus <claus@slac.stanford.edu>
# Created:      Sat Mar 16 10:52:59 2013
# Committed:  \$Date: 2013-08-14 18:28:21 -0700 (Wed, 14 Aug 2013) $ by \$Author: claus $.
# Revision:   \$Revision: 1929 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/test/networking/rules.mk $
# Credits:      SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

# Build only for the PPC based RCEs and CMs
ifeq ($(and $(have_rtems),$(or $(have_ppc),$(have_arm))),$(true))

# Append to the standard targets to make and clean the executable file
bins   := $(bindir)/netTest.elf

.PHONY: clean_netTest
cleans: clean_netTest

# For flash use: Strip the executable of extraneous symbols and sections
$(bindir)/netTest.elf: $(bindir)/netTest
	@echo "[LB] {$(tgt_arch)} Building netTest.elf"
	$(quiet)$(STRIP) -p -o $@ $<

clean_netTest:
	@echo "[RB] {$(tgt_arch)} Removing netTest, .elf, .lst and .map"
	$(quiet)rm -f $(bindir)/netTest{,.elf,.lst,.map}

endif

bin:   $(bins)

clean: $(cleans)
