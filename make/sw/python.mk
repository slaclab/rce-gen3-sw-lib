
### JHP: SIP commented out as it (almost certainly) does not hook up 
### dependencies correctly -- 12/06/12
## Call sip interpreter for python wrappers
#$(objdir)/%_sip_wrap.o: $(objdir)/%_sip_wrap.cc
#	 @echo "[CX] Compiling $<"
#	 $(quiet)$(CXX) $(incdirs_$*_sip_wrap.cc) $(DEFINES) $(CXXFLAGS) $(CFLAGS) -I $(objdir) -c $< -o $@
#
#
#$(objdir)/%_sip_wrap.cc: %_sip_wrap.sip
#	 @echo "[WG] Python sip $<"
#	 $(quiet)sip $(SIPFLAGS) -I $(RELEASE_DIR) -e -j 1 -c $(objdir)/ $<
#	 cp $(objdir)/sip$(notdir $*)part0.cc .
#	 $(quiet)mv $(objdir)/sip$(notdir $*)part0.cc $@
#


SWIG = swig
# Filter for SWIG input files (.i suffix)
getobjects_swig = $(strip \
    $(patsubst %.i,$(1)/%.o, $(filter %.i,$(2))))

# Add the new SWIG dependencies to the list
define swig_library_template
  libswigobjs_$(1) := $$(call getobjects_swig,$$(objdir),$$(libsrcs_$(1)))
  objects      += $$(libswigobjs_$(1))
  depends      += $$(libswigobjs_$(1):$$(objdir)/%.o=$$(depdir)/%.d)
  #If swigobjects are present, add the generated python file, too
  libraries += $$(if $$(libswigobjs_$(1)),$$(libdir)/$(1).py,)
$$(library_$(1)): $$(libswigobjs_$(1))
endef
$(foreach lib,$(libnames),$(eval $(call swig_library_template,$(lib))))

# Call swig interpreter for python wrappers
$(objdir)/%_swig_wrap.o: $(objdir)/%_swig_wrap.cc
	@echo "[CC] Compiling $<"
	$(quiet)$(CXX) $(incdirs_$*_swig_wrap.i) $(DEFINES) $(CXXFLAGS) $(SWIGCXXFLAGS) -c $< -o $@

# -outdir option puts generated .py file in $(libdir) to simplify $PYTHONPATH
$(objdir)/%_swig_wrap.cc: %_swig_wrap.i
	@echo "[WG] Python swig $<"
	$(quiet)$(SWIG) $(incdirs_$*_swig_wrap.i) $(SWIGFLAGS) -c++ -python -o $@ -outdir $(libdir) $<

SWGDEPSED = sed '\''s!$*\_swig_wrap_wrap.c!$(objdir)/$(notdir $*)\_swig_wrap.cc $@!g'\'' 
SWGDEP = $(SWIG) $(DEPFLAGS) $(incdirs_$*_swig_wrap.i)

$(depdir)/%_swig_wrap.d: %_swig_wrap.i
	@echo "[DI] Dependencies for $<" 
	$(quiet)$(SHELL) -ec '$(SWGDEP) $< | $(SWGDEPSED) > $@'

# Include the dependency files.  If one of the .d files in depends
# does not exist, then make invokes one of the rules [Dn] above to
# rebuild the missing .d file.  This can be short-circuited by
# defining the symbol 'no_depends'.

ifneq ($(depends),)
ifeq  ($(no_depends),)
-include $(depends)
endif
endif
