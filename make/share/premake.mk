# Prepare to run package level makefile
# -------------------------------------
Makefile:;

# This makefile is conditionally included at the start of each package
# specific Makefile. Its purpose is to correctly call itself after
# creating the appropriate directories, set tgt_arch and the target
# specific flags.

#include $(RELEASE_DIR)/make/share/arch.mk

# Symbols
# -------
# To be sure there is one, and only one, `-f Makefile' option
MAKE := $(filter-out -f Makefile, $(MAKE)) -f Makefile
PYTHON := /usr/bin/env python

# Define the make targets in terms of the architectures requested.
# Normally, the user will type "make all", but in some cases the user
# only wishes to build one specific architecture (which may or may not
# be in their normal build list).  For this case, we need to add
# $(tgt_arch) to the architecture list (sorted to remove duplicates).
archs := $(sort $(architectures) $(tgt_arch))

# Define the allowed recursive targets
recurs_targs := bin lib clean cleanall dir print doc

# Rule specific flags.  Used by arch_tgt_template(arch,tgt)
clean-flags     := no_depends=y
cleanall-flags  := no_depends=y
print-flags     := no_depends=y
install-flags   := no_depends=y
dir-flags       := no_depends=y
doc-flags       := 

# Rules
# -----
.PHONY: $(archs) all bin lib dir clean print cleanall doc

all: lib bin
lib: dir

# function arch_tgt_template
#   Define the make call executed for a specific arch/target pair
#     Note:  The construct $$($(2)-flags) references the rule specific
#     flags section above.
#   $(1) ==> an architecture
#   $(2) ==> a target
define arch_tgt_template
# make -j is problematic in the bin step, as the compilation of .so's for 
# RTEMS is highly order dependent.
ifneq ($(2),bin)
$(1).$(2):
	$$(quiet)$$(MAKE) -j4 PREMAKE_DONE=y tgt_arch=$(1) $(2) $$($(2)-flags)
else
$(1).$(2):
	$$(quiet)$$(MAKE) PREMAKE_DONE=y tgt_arch=$(1) $(2) $$($(2)-flags)
endif
endef

# function arch_template
#   Defines the architecture dependent targets (architecture depends on
#     dir/lib/bin portions)
#   $(1) ==> an architecture
#   Calls arch_tgt_template
define arch_template
$(foreach tgt,$(recurs_targs),$(eval $(call arch_tgt_template,$(1),$(tgt))))
$(1): $(1).dir $(1).lib $(1).bin
endef
$(foreach arc,$(archs),$(eval $(call arch_template,$(arc))))

# function tgt_template
#   Defines the dependencies for targets, dependent on whether or not
#     tgt_arch is defined.  Result targets/deps look like this:
#     lib: i386-linux.lib
#   $(1) ==> a target
define tgt_template
ifeq ($(tgt_arch),)
$(1): $(addsuffix .$(1),$(archs))
else
$(1): $(tgt_arch).$(1)
endif
endef
$(foreach tgt,$(recurs_targs),$(eval $(call tgt_template,$(tgt))))


