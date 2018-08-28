# Package level makefile
# ----------------------
Makefile:;

include $(RELEASE_DIR)/make/share/arch.mk

#------------------------------------------------------------------------
# Symbol definitions
#------------------------------------------------------------------------
SHELL := /bin/bash
RM    := rm -f
MV    := mv -f
CP    := cp
empty :=
space := $(empty) $(empty)

#------------------------------------------------------------------------
# Define the package and project names in terms of the current working directory.
# This requires the standard <release>/<project>/<package> code layout.
pwd := $(shell pwd)
cwd := $(call reverse,$(subst /, ,$(pwd)))
pkg_name := $(word 1,$(cwd))
prj_name := $(word 2,$(cwd))

# The base directories where built code ends up
buildDir := $(RELEASE_DIR)/build

# Defines which directories are being created by this makefile for generated
# dependencies, libraries, modules, objects, and binaries.
libdir     := $(buildDir)/$(tgt_arch)/lib
bindir     := $(buildDir)/$(tgt_arch)/bin
objdir     := $(buildDir)/$(tgt_arch)/obj/$(prj_name)/$(pkg_name)
moddir     := $(buildDir)/$(tgt_arch)/mod
depdir     := $(buildDir)/$(tgt_arch)/dep/$(prj_name)/$(pkg_name)

#------------------------------------------------------------------------
# Prerequisite variables used for broad make targets such as 'all',
# 'obj', 'lib', 'dir', etc.
#------------------------------------------------------------------------
libraries :=
modules   :=
targets   :=
objects   :=
depends   :=
# note: prod_dirs is static, depending only on the project/package names.
#       temp_dirs is dynamic, and depends on the exact mix of dependency
#         fragments and object files
prod_dirs := $(strip $(bindir) $(libdir) $(moddir) $(depdir))
temp_dirs  = $(strip $(sort $(foreach o,$(depends) $(objects),$(dir $(o)))))

#------------------------------------------------------------------------
# Procedures for creating rules for libraries and targets.
#
# Note:  $(eval ...) needs gmake >= 3.80 which is checked for in setup.mk
#------------------------------------------------------------------------

# generate a list of .o files in directory $(1) from a list of src files $(2)
getobjects = $(strip \
	$(patsubst %.cc,$(1)/%.o,  $(filter %.cc,$(2))) \
	$(patsubst %.cpp,$(1)/%.o, $(filter %.cpp,$(2))) \
	$(patsubst %.c,$(1)/%.o,   $(filter %.c,$(2))) \
	$(patsubst %.s,$(1)/%.o,   $(filter %.s,$(2))) \
	$(patsubst %.S,$(1)/%.o,   $(filter %.S,$(2))))

#
getprj = $(word 1,$(subst /, ,$(1)))
getlib = $(word 2,$(subst /, ,$(1)))


getlibrary = $(libdir)/lib$(call getlib,$(1)).$(LIBEXTNS)

getlibraries = $(foreach prjlib,$(1),$(call getlibrary,$(prjlib)))

getprojects  = $(foreach prjlib,$(1),$(call getprj,$(prjlib)))

getlinkdirs  = $(addprefix -L, $(sort $(foreach prj,$(call getprojects,$(1)),$(libdir))))

getlinksdir  = $(addprefix -L, $(sort $(dir $(1))))

getlinklibs  = $(addprefix -l,$(foreach prjlib,$(1),$(call getlib,$(prjlib))))

getbuildlibs = $(foreach prjlib,$(1),$(call getlibrary,$(prjlib)))

getlinkslib  = $(addprefix -l,$(notdir $(1)))

getrpath  = $$ORIGIN/../lib

#getrpaths = $(subst $(space),:,$(strip $(foreach prj,$(call getprojects,$(1)),$(call getrpath,$(prj)))))

getmodname = $(word 1,$(subst ., ,$(1)))

ifndef DAT_RELEASE
getbasemod = $(moddir)/$(call getlib,$(1)).$(SHLIBEXTNS)
else
getbasemod = $(call getlib,$(1)).$(SHLIBEXTNS)
endif

getbasemods = $(foreach mod,$(1),$(call getbasemod,$(mod)))

getbasemodflags = $(addprefix -l:, $(call getbasemods,$(1)))

ifndef DAT_RELEASE
getbasetgt = $(bindir)/$(call getlib,$(1)).$(SHLIBEXTNS)
else
getbasetgt = $(call getlib,$(1)).$(SHLIBEXTNS)
endif

getbasetgts = $(foreach tgt,$(1),$(call getbasetgt,$(tgt)))

getbasetgtflags = $(addprefix -l:, $(call getbasetgts,$(1)))

getbasetgtpath = $(bindir)

getbasetgtpaths = $(foreach tgt,$(1),$(call getbasetgtpath,$(tgt)))

getbasetgtpathflags = $(addprefix -L, $(call getbasetgtpaths,$(1)))

getbasemodpath = $(moddir)

getbasemodpaths = $(foreach tgt,$(1),$(call getbasemodpath,$(tgt)))

getbasemodpathflags = $(addprefix -L, $(call getbasemodpaths,$(1)))

upcase = $(shell echo $(1)|tr a-z A-Z)


define object_template
#  incdirs_$(1) := $$(addprefix -I$(RELEASE_DIR)/,$(2))
#  incdirs_$(1) += -I$(RELEASE_DIR)
  incdirs_$(1) := -I$(incfarm)
ifeq ($(tgt_os),rtems)
  incdirs_$(1) += -isystem $(RTEMS_INCLUDE)
endif
  incdirs_$(1) += $$(addprefix -I,$(3))
# optionally add the DAT_RELEASE to the incdirs
ifdef DAT_RELEASE
  incdirs_$(1) += -I$(DAT_RELEASE)/include/rce
ifeq ($(tgt_os),rtems)
  incdirs_$(1) += -isystem $(DAT_RELEASE)/include/rtems
endif
endif
endef

#------------------------------------------------------------------
# A library is a simple, non-dynamically loaded library
#------------------------------------------------------------------
define library_template
  library_$(1) := $$(libdir)/lib$(1).$(LIBEXTNS)
  libobjs_$(1) := $$(call getobjects,$$(objdir),$$(libsrcs_$(1)))
  libraries    += $$(library_$(1))
  objects      += $$(libobjs_$(1))
  depends      += $$(libobjs_$(1):$$(objdir)/%.o=$$(depdir)/%.d)
  libraries_$(1) := $$(call getlibraries,$$(liblibs_$(1)))
  linkdirs_$(1)  := $$(call getlinkdirs,$$(liblibs_$(1)))
  linkdirs_$(1)  += $$(call getlinksdir,$$(libslib_$(1)))
ifdef DAT_RELEASE
  linkdirs_$(1) += -L$(DAT_RELEASE)/lib
endif
ifneq ($$(liblibs_$(1)),)
  linklibs_$(1)  := $$(call reverse,$$(call getlinklibs,$$(liblibs_$(1))))
endif
ifneq ($$(libslib_$(1)),)
  linklibs_$(1)  += $$(call reverse,$$(call getlinkslib,$$(libslib_$(1))))
endif
ifneq ($$(librlib_$(1)),)
  linklibs_$(1)  += $$(call reverse,$$(call getlinklibs,$$(librlib_$(1))))
endif
ifeq ($$(LIBEXTNS),so)
  rpaths_$(1)    := -Wl,-rpath='$$(call getrpath)'
ifneq ($$(ifversn_$(1)),)
  ifversnflags_$(1) := -Wl,--version-script=$$(ifversn_$(1))
endif
endif
  linkflags_$(1) := $$(linkdirs_$(1)) $$(linklibs_$(1)) $$(rpaths_$(1))
$$(library_$(1)): $$(libobjs_$(1))
endef

$(foreach lib,$(libnames),$(eval $(call library_template,$(lib))))
$(foreach lib,$(libnames),$(foreach obj,$(libsrcs_$(lib)),$(eval $(call object_template,$(obj),$(libincs_$(lib)),$(libsinc_$(lib))))))

#------------------------------------------------------------------
# A target is an executable either loaded by the bootstrap loader
# or is run from the command line on an OS such as linux
#------------------------------------------------------------------
define target_template
  target_$(1)  := $$(bindir)/$(1)
  tgtobjs_$(1) := $$(call getobjects,$$(objdir),$$(tgtsrcs_$(1)))
  targets      += $$(target_$(1))
  objects      += $$(tgtobjs_$(1))
  depends      += $$(tgtobjs_$(1):$$(objdir)/%.o=$$(depdir)/%.d)
#ifndef DAT_RELEASE
  libraries_$(1) := $$(call getlibraries,$$(tgtlibs_$(1)))
#else
#  libraries_$(1) := $$(call getlibraries,$$(filter $$(prj_name)/%,$$(tgtlibs_$(1))))
#endif
ifdef DAT_RELEASE
  linkdirs_$(1) += -L$(DAT_RELEASE)/lib/$(tgt_arch)
endif
ifneq ($$(tgtlibs_$(1)),)
ifneq ($(tgt_os),rtems)
  linkdirs_$(1)  := $$(call getlinkdirs,$$(tgtlibs_$(1)))
  linklibs_$(1)  := $$(call reverse,$$(call getlinklibs,$$(tgtlibs_$(1))))
else
  linklibs_$(1)  := $$(call reverse,$$(call getbuildlibs,$$(tgtlibs_$(1))))
endif
endif
ifneq ($$(tgtslib_$(1)),)
  linkdirs_$(1)  += $$(call getlinksdir,$$(tgtslib_$(1)))
  linklibs_$(1)  += $$(call reverse,$$(call getlinkslib,$$(tgtslib_$(1))))
endif
ifneq ($$(tgtrlib_$(1)),)
  linklibs_$(1)  += $$(call reverse,$$(call getlinklibs,$$(tgtrlib_$(1))))
endif
ifeq ($$(LIBEXTNS),so)
  rpaths_$(1)    := -Wl,-rpath='$$(call getrpath)'
endif
  linkflags_$(1) := $$(linkdirs_$(1)) $$(linklibs_$(1)) $$(rpaths_$(1))
$$(target_$(1)): $$(tgtobjs_$(1)) $$(libraries_$(1))
endef

$(foreach tgt,$(tgtnames),$(eval $(call target_template,$(tgt))))
$(foreach tgt,$(tgtnames),$(foreach obj,$(tgtsrcs_$(tgt)),$(eval $(call object_template,$(obj),$(tgtincs_$(tgt)),$(tgtsinc_$(tgt))))))

#------------------------------------------------------------------ 
# A module is a dynamically loadable shared library containing code
# which may be loaded by a target
#------------------------------------------------------------------
define module_template
  module_$(1) := $$(moddir)/$(1).so
  modobjs_$(1) := $$(call getobjects,$$(objdir),$$(modsrcs_$(1)))
  modules      += $$(module_$(1))
  objects      += $$(modobjs_$(1))
  depends      += $$(modobjs_$(1):$$(objdir)/%.o=$$(depdir)/%.d)
#ifndef DAT_RELEASE
  libraries_$(1) := $$(call getlibraries,$$(modlibs_$(1)))
#else
  # When we're working with a DAT release, we don't want to add hard dependencies on non-project libraries
#  libraries_$(1) := $$(call getlibraries,$$(filter $$(prj_name)/%,$$(modlibs_$(1))))
#endif
  libraries_$(1) += $$(modslibw_$(1))
ifdef DAT_RELEASE
  linkflags_$(1) += $$(call getbasetgtpathflags,$(basetgt_$(1)))
  linkflags_$(1) += $$(call getbasemodpathflags,$(basemod_$(1)))
  linkdirs_$(1) += -L$(DAT_RELEASE)/lib
endif
  linkflags_$(1) += $$(linkdirs_$(1)) $$(linklibs_$(1))
ifeq ($(tgt_os),rtems)
  linkflags_$(1) += $$(call getbasetgtflags,$(basetgt_$(1)))
  linkflags_$(1) += $$(call getbasemodflags,$(basemod_$(1)))
endif
$$(module_$(1)): $$(modobjs_$(1)) $$(libraries_$(1))
ifndef DAT_RELEASE
$$(module_$(1)): $$(call getbasetgts,$(basetgt_$(1))) $$(call getbasemods,$(basemod_$(1)))
endif
ifndef namespace_$1
   namespace_$1 := $(NAMESPACE)
endif
endef

$(foreach mod,$(modnames),$(eval $(call module_template,$(mod))))
$(foreach mod,$(modnames),$(foreach obj,$(modsrcs_$(mod)),$(eval $(call object_template,$(obj),$(modincs_$(mod)),$(modsinc_$(mod))))))


define svt_template
  svt_$(1)     := $$(moddir)/$(1).svt
  svtobjs_$(1) := $$(call getobjects,$$(objdir),$$(svtsrcs_$(1)))
  svts         += $$(svt_$(1))
  objects      += $$(svtobjs_$(1))
  libraries_$(1) := $$(call getlibraries,$$(svtlibs_$(1)))
ifdef DAT_RELEASE
  linkflags_$(1) += $$(call getbasetgtpathflags,$(basetgt_$(1)))
  linkflags_$(1) += $$(call getbasemodpathflags,$(basemod_$(1)))
  linkflags_$(1) += -L$(DAT_RELEASE)/lib
else
  linkflags_$(1) += -L$(RELEASE_DIR)/build/$(tgt_arch)/lib
  linklibraries_$(1) += $$(call getbasetgts,$(basetgt_$(1)))
  linklibraries_$(1) += $$(call getbasemods,$(basemod_$(1)))
endif
  linkflags_$(1) += $$(call getbasetgtflags,$(basetgt_$(1)))
  linkflags_$(1) += $$(call getbasemodflags,$(basemod_$(1)))
$$(svt_$(1)): $$(svtobjs_$(1)) $$(libraries_$(1))
ifndef namespace_$1
   namespace_$1 := $(NAMESPACE)
endif
endef
$(foreach svt,$(svtnames),$(eval $(call svt_template,$(svt))))
$(foreach svt,$(svtnames),$(foreach obj,$(svtsrcs_$(svt)),$(eval $(call object_template,$(obj),$(svtincs_$(svt)),$(svtsinc_$(svt))))))


define task_template
  task_$(1)     := $$(moddir)/$(1).exe
  taskobjs_$(1) := $$(call getobjects,$$(objdir),$$(tasksrcs_$(1)))
  tasks         += $$(task_$(1))
  objects      += $$(taskobjs_$(1))
  libraries_$(1) := $$(call getlibraries,$$(tasklibs_$(1)))
ifdef DAT_RELEASE
  linkflags_$(1) += $$(call getbasetgtpathflags,$(basetgt_$(1)))
  linkflags_$(1) += $$(call getbasemodpathflags,$(basemod_$(1)))
  linkflags_$(1) += -L$(DAT_RELEASE)/lib
else
  linkflags_$(1) += -L$(RELEASE_DIR)/build/$(tgt_arch)/lib
  linklibraries_$(1) += $$(call getbasetgts,$(basetgt_$(1)))
  linklibraries_$(1) += $$(call getbasemods,$(basemod_$(1)))
endif
  linkflags_$(1) += $$(call getbasetgtflags,$(basetgt_$(1)))
  linkflags_$(1) += $$(call getbasemodflags,$(basemod_$(1)))
$$(task_$(1)): $$(taskobjs_$(1)) $$(libraries_$(1)) $$(linklibraries_$(1))
ifndef namespace_$1
   namespace_$1 := $(NAMESPACE)
endif
endef
$(foreach task,$(tasknames),$(eval $(call task_template,$(task))))
$(foreach task,$(tasknames),$(foreach obj,$(tasksrcs_$(task)),$(eval $(call object_template,$(obj),$(taskincs_$(task)),$(tasksinc_$(task))))))

# Rules
# -----
rules := all dir obj lib bin clean cleanall print \
         install installdirs

.PHONY: $(rules) $(libnames) $(tgtnames)

.SUFFIXES:  # Kills all implicit rules

all: bin;

obj: $(objects);

lib: dir $(libraries);

bin: lib $(targets) $(modules) $(svts) $(tasks);

dir: $(incfarm) $(prod_dirs) $(temp_dirs);

print:
	@echo	"bindir    = $(bindir)"
	@echo	"moddir    = $(moddir)"
	@echo	"libdir    = $(libdir)"
	@echo	"objdir    = $(objdir)"
	@echo	"modobjdir = $(modobjdir)"
	@echo	"depdir    = $(depdir)"
	@echo   "targets   = $(targets)"
	@echo	"libraries = $(libraries)"
	@echo   "modules   = $(modules)"
	@echo	"depends   = $(depends)"
	@echo	"objects   = $(objects)"

clean:
ifneq ($(objects),)
	@echo "[RO] {$(tgt_arch)} Removing object files"
	$(quiet)$(RM) $(objects)
endif
ifneq ($(depends),)
	@echo "[RD] {$(tgt_arch)} Removing depend files"
	$(quiet)$(RM) $(depends)
endif
ifneq ($(libraries),)
	@echo "[RL] {$(tgt_arch)} Removing libraries: $(notdir $(libraries))"
	$(quiet)$(RM) $(libraries)
endif
ifneq ($(targets),)
	@echo "[RT] {$(tgt_arch)} Removing targets: $(notdir $(targets))"
	$(quiet)$(RM) $(targets)
endif
ifneq ($(modules),)
	@echo "[RM] {$(tgt_arch)} Removing modules: $(notdir $(modules))"
	$(quiet)$(RM) $(modules)
endif

cleanall:
	$(quiet)$(RM) -r $(temp_dirs)

alldirs = $(prod_dirs) $(temp_dirs)
# Directory structure
$(alldirs):
	mkdir -p $@

# Libraries
$(libdir)/lib%.$(LIBEXTNS):
	@echo "[LD] {$(tgt_arch)} Build library $*"
	$(quiet)$(LD) $(LDFLAGS) $(ifversnflags_$*) $(linkflags_$*) $^ -o $@


# Executables
$(bindir)/%:
	@echo "[LT] {$(tgt_arch)} Linking target $*"
	$(quiet)$(LX) $(LXFLAGS) $(tgtobjs_$*) $(linkflags_$*) -Wl,-Map,$(bindir)/$*.map -o $@


# Modules
$(moddir)/%.so:
	@echo "[LM] {$(tgt_arch)} Linking module $*"
	$(MLX) -- $(CXXFLAGS)\
	    -Wl,--whole-archive $(libraries_$*) \
	    $(MLXFLAGS) \
	    -Wl,-soname,$(namespace_$*):$*.$(SHLIBEXTNS) \
	    -L $(RELEASE_DIR)/make/sw/$(tgt_cpu_family) \
	    $(incdirs_$*) \
	    $(modobjs_$*) \
            $(linkflags_$*) \
	    -Wl,-Map,$(@).map \
	    -o $@

# SVTs
$(moddir)/%.svt:
	@echo "[LSVT] {$(tgt_arch)} Linking SVT $*"
	rtems-svt -- $(CFLAGS) \
            $(svtobjs_$(*)) \
	    $(DEFINES) \
	    -Wl,-soname,$(namespace_$*):$*.$(SVTEXTNS) \
	    -Wl,--whole-archive $(libraries_$*) \
	    -L $(RELEASE_DIR)/make/sw/$(tgt_cpu_family) \
	    $(incdirs_$*) \
	    $(linkflags_$*) \
	    -Wl,-Map,$(@).map \
	    -o $@

# Tasks (RTEMS Only)
$(moddir)/%.exe:
	@echo "[LTASK] {$(tgt_arch)} Linking Task $*"
	rtems-task -- $(CFLAGS) \
            $(taskobjs_$(*)) \
	    $(DEFINES) \
	    -Wl,-soname,$(namespace_$*):$*.$(TASKEXTNS) \
	    -Wl,--whole-archive $(libraries_$*) \
	    -L $(RELEASE_DIR)/make/sw/$(tgt_cpu_family) \
	    $(incdirs_$*) \
	    $(linkflags_$*) \
	    -Wl,-Map,$(@).map \
	    -o $@

# Objects for C, C++ and assembly files
$(objdir)/%.o: %.c
	@echo "[CC] {$(tgt_arch)} Compiling $<"
	$(quiet)$(CC) $(abspath $<) $(CFLAGS) -c $(incdirs_$<) $(CPPFLAGS) $(DEFINES) -o $@

$(objdir)/%.o: %.cc
	@echo "[CX] {$(tgt_arch)} Compiling $<"
	$(quiet)$(CXX) $(abspath $<) $(CXXFLAGS) -c $(incdirs_$<) $(CPPFLAGS) $(DEFINES) -o $@

$(objdir)/%.o: %.cpp
	@echo "[CX] {$(tgt_arch)} Compiling $<"
	$(quiet)$(CXX) $(abspath $<) $(CXXFLAGS) -c $(incdirs_$<) $(CPPFLAGS) $(DEFINES) -o $@

$(objdir)/%.o: %.s
	@echo "[CS] {$(tgt_arch)} Compiling $<"
	$(quiet)$(CXX) $(CASFLAGS) $(abspath $<) $(CFLAGS) -c $(incdirs_$<) $(CPPFLAGS) $(DEFINES) -o $@

$(objdir)/%.o: %.S
	@echo "[CS] {$(tgt_arch)} Compiling $<"
	$(quiet)$(CXX) $(CASFLAGS) $(abspath $<) $(CFLAGS) -c $(incdirs_$<) $(CPPFLAGS) $(DEFINES) -o $@


#ifneq ($(depends),)
#$(depends): $(depdir);
#endif

# Defines rules to (re)build dependency files
DEPSED = sed "s!$(notdir $*)\.o!$(objdir)/$*\.o $@!g"
CXXDEP = $(CPPFLAGS) $(DEFINES) $(DEPFLAGS) $(incdirs_$<)
CCDEP  = $(CPPFLAGS) $(DEFINES) $(DEPFLAGS) $(incdirs_$<)

$(depdir)/%.d: %.c
	@echo "[DC] {$(tgt_arch)} Dependencies for $<"
	if ! test -d $(dir $@); then \
	  mkdir -p $(dir $@); \
	fi
	$(CC) $< $(CFLAGS) $(CCDEP) | $(DEPSED)  > $@

$(depdir)/%.d: %.cc
	@echo "[DX] {$(tgt_arch)} Dependencies for $<"
	if ! test -d $(dir $@); then \
	  mkdir -p $(dir $@); \
	fi
	$(CXX) $< $(CXXFLAGS) $(CXXDEP) | $(DEPSED) > $@

$(depdir)/%.d: %.cpp
	@echo "[DX] {$(tgt_arch)} Dependencies for $<"
	if ! test -d $(dir $@); then \
	  mkdir -p $(dir $@); \
	fi
	$(CXX) $< $(CXXFLAGS) $(CXXDEP) | $(DEPSED) > $@

$(depdir)/%.d: %.s
	@echo "[DS] {$(tgt_arch)} Dependencies for $<"
	if ! test -d $(dir $@); then \
	  mkdir -p $(dir $@); \
	fi
	$(CC) $< $(CPPFLAGS) $(CCDEP) | $(DEPSED) > $@

$(depdir)/%.d: %.S
	@echo "[DS] {$(tgt_arch)} Dependencies for $<"
	if ! test -d $(dir $@); then \
	  mkdir -p $(dir $@); \
	fi
	$(CC) $< $(CPPFLAGS) $(CCDEP) | $(DEPSED) > $@

# Include the doxygen rules which uses variables from above
include $(RELEASE_DIR)/make/share/doxygen.mk


# Include the dependency files.  If one of the .d files in depends
# does not exist, then make invokes one of the rules [Dn] above to
# rebuild the missing .d file.  This can be short-circuited by
# defining the symbol 'no_depends', as is done in premake.mk.

ifneq ($(depends),)
ifeq  ($(no_depends),)
-include $(depends)
endif
endif
