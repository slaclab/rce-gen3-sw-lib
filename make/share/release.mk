# Top level makefile
# ------------------
Makefile:;

# Symbols
# -------
SHELL := /bin/bash

# Minimal directory tree
buildDir = $(RELEASE_DIR)/build
tree_dirs := $(buildDir)

# Define the allowed recursive targets
recurs_targs := bin lib clean doc

# Rules
# -----
.PHONY: dir cleanall all bin lib include doc

# function release_project_template
#   Defines the rule <project>.<tgt> (like core.lib or tool.clean)
#   $(1) ==> a project
#   $(2) ==> the target (bin/lib/clean)
define release_project_template
.PHONY: $(1).$(2)
$(1).$(2):
	@echo "[PR] Project <`printf %-13s $(1)`> Target <`printf %-3s $(2)`> {$(tgt_arch)}"
	$$(MAKE) -C $(1) $(2)
$(1).$(2):
endef
$(foreach prj,$(projects),$(foreach targ,$(recurs_targs),$(eval $(call release_project_template,$(prj),$(targ)))))

# function release_arch_template
#   Defines the rule <architecture> which builds bin and lib for <architecture>
#   $(1) ==> an architecture (arm-rtems-rceCA9-dbg, etc)
define release_arch_template
$(1): dir $(1).lib $(1).bin
$(1).doc: 
	$$(MAKE) doc tgt_arch=$(1)
$(1).lib: 
	$$(MAKE) lib tgt_arch=$(1)
$(1).bin:
	$$(MAKE) bin tgt_arch=$(1)
$(1).clean:
	@$(RM) -rf $(RELEASE_DIR)/build/$(tgt_arch)
endef
$(foreach arch,$(architectures),$(eval $(call release_arch_template,$(arch))))

#lib: $(incfarm)

# bin and lib have different meanings whether they are called with tgt_arch
# defined or not.  If tgt_arch is not defined, we loop over architectures
# and build the target.  If it is defined, build <project>.<tgt> for all projects.
ifeq ($(tgt_arch),)
doc: $(addsuffix .doc,$(architectures))
lib: $(addsuffix .lib,$(architectures))
bin: $(addsuffix .bin,$(architectures))
else
doc: $(addsuffix .doc,$(projects))
lib: $(addsuffix .lib,$(projects))
bin: $(addsuffix .bin,$(projects))
endif

all: dir lib bin

dir: $(tree_dirs)
$(tree_dirs):
	$(quiet)mkdir -p $@


include: dir
	@echo "[IF] Top level include farm"
	for arch in $(architectures); do \
	  /usr/local/bin/python make/tools/includeFarm.py --arch=$$arch \
	     --dest=build/$$arch/include/core; \
	done;

cleanall:
	@echo "[CL] Target <cleanall> at top level";
	$(quiet)$(RM) -r build/*
	$(quiet)$(RM) -r $(install_base)

clean:
	@echo "[CL] Target <clean> at top level";
	@for arch in $(architectures); do \
	  $(MAKE) $$arch.clean;      \
	done

