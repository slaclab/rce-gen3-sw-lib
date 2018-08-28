# Call omniidl to preprocess .idl files
$(objdir)/%SK.o: $(objdir)/%SK.cc
	@echo "[CX] Compiling $<"
	$(quiet)$(CXX) $(incdirs_$*SK.cc) $(DEFINES) $(CPPFLAGS) $(CFLAGS) -c $< -o $@


$(objdir)/%SK.cc $(objdir)/%.hh: %.idl
	@echo "[ID] IDL Preprocessing $<"
	$(quiet)omniidl $(incdirs_$*SK.cc) $(IDLFLAGS) -bcxx  -C$(objdir)/$(dir $*) $<




