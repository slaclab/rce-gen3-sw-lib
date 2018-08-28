# Project level makefile
# ----------------------
Makefile:;

#include $(RELEASE_DIR)/make/share/arch.mk

# Symbols
# -------
cwd := $(subst /, ,$(shell pwd))
prj_name := $(word $(words $(cwd)),$(cwd))

# Define the make targets in terms of the architectures requested.
# Normally, the user will type "make all", but in some cases the user
# only wishes to build one specific architecture (which may or may not
# be in their normal build list).  For this case, we need to add
# $(tgt_arch) to the architecture list (sorted to remove duplicates).
archs := $(sort $(architectures) $(tgt_arch))

# Define the allowed recursive targets
recurs_targs := bin lib clean doc

# Rules
# -----
.PHONY: $(archs) $(packages) all lib bin clean doc

# function project_package_template
#   Defines the rule <package>.<tgt>
#   $(1) ==> a package
#   $(2) ==> the target (bin/lib/clean)
define project_package_template
$(1).$(2):
	$$(MAKE) -C $(1) $(2) tgt_arch=$(tgt_arch)
endef
$(foreach pkg,$(packages),$(foreach targ,$(recurs_targs),$(eval $(call project_package_template,$(pkg),$(targ)))))

# function project_arch_template
#   Defines the rule <architecture> which builds bin and lib for <architecture>
#   $(1) ==> an architecture (arm-rtems-rceCA9-dbg, etc)
define project_arch_template
$(1): $(1).lib $(1).bin
$(1).doc:
	$$(MAKE) doc tgt_arch=$(1)
$(1).lib:
	$$(MAKE) lib tgt_arch=$(1)
$(1).bin:
	$$(MAKE) bin tgt_arch=$(1)
endef
$(foreach arch,$(archs),$(eval $(call project_arch_template,$(arch))))

# If we have tgt_arch defined, don't add dependencies for other
# architectures, as this is the second time through
ifeq ($(tgt_arch),)
doc: $(addsuffix .doc,$(architectures))
lib: $(addsuffix .lib,$(architectures))
bin: $(addsuffix .bin,$(architectures))
else
doc: $(addsuffix .doc,$(packages))
lib: $(addsuffix .lib,$(packages))
bin: $(addsuffix .bin,$(packages))
$(tgt_arch).clean: $(addsuffix .clean,$(packages));
endif


clean:
	@echo "[RO] Removing project $(prj_name) build directory"
	$(quiet)$(RM) -r $(RELEASE_DIR)/build/$(prj_name)

all: lib bin

%:;
	@echo "---- No target <$*> for project <$(prj_name)>  <$(tgt_arch)>"

