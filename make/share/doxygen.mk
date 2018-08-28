# -*-Mode: Makefile;-*-
#
#                               Copyright 2014
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Abstract: Makefile fragment for Doxygen rules

# This file is included at the end of the relevant package.mk (hw/sw)

MKDOCS_PY := $(RELEASE_DIR)/make/tools/mkdocs.py

.PHONY: doc_usr doc_usr doc

__document := $(true)
# Do not document stuff with no operating system
ifeq ($(have_noose),$(true))
  __document = $(false)
endif

# Do not document stuff which should not be documented
# This flag would be set in a constituents file when
# no docs are ever wanted.
ifeq ($(do_not_document),$(true))
  __document = $(false)
endif


ifeq ($(__document),$(true))

docsrcs:= $(docsrcs_nolibrary)
nodocsrcs:=
devdocsrcs:=$(devdocsrcs_nolibrary)

# Aggregate components for simpler templates below
doc_items := $(libnames) $(tgtnames) $(modnames) $(svtnames) $(tasknames)

# Doxygen item template
define doxygen_item_template
  docsrcs    += $(docsrcs_$(1))
  nodocsrcs  += $(nodocs_$(1))
  nodevdocsrcs  += $(nodevdocs_$(1))
  devdocsrcs += $(devdocsrcs_$(1))
endef
$(foreach item,$(doc_items),$(eval $(call doxygen_item_template,$(item))))

# Doxygen library template
define doxygen_lib_template
  docsrcs    += $(libsrcs_$(1))
endef
$(foreach lib,$(libnames),$(eval $(call doxygen_lib_template,$(lib))))

# Doxygen module template
define doxygen_mod_template
  docsrcs    += $(modsrcs_$(1))
endef
$(foreach mod,$(modnames),$(eval $(call doxygen_mod_template,$(mod))))

# Doxygen target template
define doxygen_tgt_template
  docsrcs    += $(tgtsrcs_$(1))
endef
$(foreach tgt,$(tgtnames),$(eval $(call doxygen_mod_template,$(tgt))))

# Doxygen svt template
define doxygen_svt_template
  docsrcs    += $(modsrcs_$(1))
endef
$(foreach svt,$(svtnames),$(eval $(call doxygen_svt_template,$(svt))))

# Doxygen task template
define doxygen_task_template
  docsrcs    += $(modsrcs_$(1))
endef
$(foreach task,$(tasknames),$(eval $(call doxygen_task_template,$(task))))

doxy_flags_common := --arch $(tgt_arch)
doxy_flags_common += --source $(docsrcs)
doxy_flags_common += --exclude $(nodocsrcs)

ifneq ($(SDK_VERSION),)
doxy_flags_common += --sdkVer $(SDK_VERSION)
endif

ifneq ($(doxy_pkg_c_only),)
doxy_flags_common += -c
endif


doc_usr:
	$(quiet)echo "[DOC_USR] {$(tgt_arch)} $(prj_name)/$(pkg_name)"
ifneq ($(strip $(docsrcs)),)
	$(quiet)python $(MKDOCS_PY) $(doxy_flags_common)
else
	$(quiet)echo "No public doc srcs for $(prj_name)/$(pkg_name) {$(tgt_arch)}"
endif

doc_dev: 
	$(quiet)echo "[DOC_DEV] {$(tgt_arch)} $(prj_name)/$(pkg_name)"
ifneq ($(strip $(docsrcs) $(devdocsrcs)),)
	$(quiet)python $(MKDOCS_PY) $(doxy_flags_common) \
	   --devSource $(devdocsrcs) \
	   --devExclude   $(nodevdocsrcs) \
	   --developer
else
	$(quiet)echo "No development doc srcs for $(prj_name)/$(pkg_name) {$(tgt_arch)}"
endif


doc: doc_usr doc_dev

else
# do_not_document == true

doc:

endif