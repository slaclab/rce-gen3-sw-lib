# -*-Mode: Makefile;-*-
#
# Facility:     DAT
# File:         rules.mk
# Abstract:     Special make rules for the DBI (DAT Basic Infrastructure code).
# Author:       Steve Tether <tether@slac.stanford.edu>
# Created:      2011/05/18
# Committed:  \$Date: 2015-07-14 10:55:53 -0700 (Tue, 14 Jul 2015) $ by \$Author: smaldona $.
# Revision:   \$Revision: 4078 $
# Repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/platform/startup/rules.mk $
# Credits:      SLAC
#                              Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

# Build U-Boot header wrapped images only for the ARM based boards
ifeq ($(and $(have_rtems),$(have_zynq)),$(true))

ver  := $(dbi_majorv).$(dbi_minorv)

# Append to the standard "bin" target to make the elf file
bins += $(bindir)/urtems.elf

.PHONY: clean_udbi
clean:  clean_udbi

# Produce the U-Boot header wrapped image
$(bindir)/urtems.elf: $(bindir)/rtems.${SHLIBEXTNS}
	@echo "[LB] {$(tgt_arch)} Generating urtems.elf"
	$(quiet)$(MKIMAGE) -a 0x3000040 -e 0x104200 \
	   -C None -A arm -T kernel -O rtems -n rtems.${SHLIBEXTNS} -d $< $@


# Override the RTEMS BSP hooks with our own
LDWRAPS := -Wl,--wrap=bsp_start
LDWRAPS += -Wl,--wrap=bsp_pretasking_hook
LDWRAPS += -Wl,--wrap=bsp_predriver_hook
LDWRAPS += -Wl,--wrap=bsp_postdriver_hook

# snarf all libraries
LDALLLIBS  += -Wl,--whole-archive

# RTEMS needs the C and C++ Runtime
GNU_OBJECTS   := $(GNU_LIBS)/libc.a        
GNU_OBJECTS   += $(GNU_LIBS)/libgcc.a      
GNU_OBJECTS   += $(GNU_LIBS)/libstdc++.a   

# RTEMS needs the crt objects
CRT_DIRECTORY := $(GNU_LIBS)
CRT_OBJECTS   := $(CRT_DIRECTORY)/crti.o $(CRT_DIRECTORY)/crtbegin.o $(CRT_DIRECTORY)/crtend.o $(CRT_DIRECTORY)/crtn.o

# But only part of the math library
RTEMS_LIBM := -Wl,--no-whole-archive -lm

# and start.o from RTEMS
RTEMS_START := $(RTEMS_LIBS)/start.o

# and a whole set of libraries
rlibs := libblock.a libbsp.a libcpuuse.a libcsupport.a libi2c.a
rlibs += libdefaultfs.a libdevnull.a libdosfs.a libfsmount.a libimfs.a
rlibs += libposix.a librfs.a libmd.a 
rlibs += librtems.a libsapi.a libscore.a libscorecpu.a
rlibs += libserialio.a libstackchk.a libstringto.a libsupport.a
rlibs += libuntar.a libutf8proc.a libuuid.a libxdr.a libz.a
RTEMS_LIBRARIES := $(addprefix $(RTEMS_LIBS)/,$(rlibs))

# One generated link file required for sysimg.ld
bins += $(bindir)/memory.ld
$(bindir)/rtems.${SHLIBEXTNS}:  $(bindir)/memory.ld
$(bindir)/memory.ld: $(RELEASE_DIR)/tool/memory/armCA9/memRegionConstants.h
$(bindir)/memory.ld: $(RELEASE_DIR)/make/sw/arm/rceCA9/memory.ld.in
$(bindir)/memory.ld:
	$(quiet)$(CPP) -imacros $(RELEASE_DIR)/tool/memory/armCA9/memRegionConstants.h \
	  -P -C -nostdinc \
	   $(RELEASE_DIR)/make/sw/arm/rceCA9/memory.ld.in \
	   -o $@

# Copy in shareable.ld from the make directory for use by SDK
bins += $(bindir)/shareable.ld
$(bindir)/shareable.ld: $(RELEASE_DIR)/make/sw/arm/shareable.ld
	$(quiet)/bin/cp $< $@


# The link script for RTEMS is mostly set, but can be overridden
LSCRIPT   = -L$(RELEASE_DIR)/make/sw/$(tgt_cpu_family)
LSCRIPT  += -L$(RELEASE_DIR)/make/sw/$(tgt_cpu_family)/$(tgt_board)
LSCRIPT  += -L$(RELEASE_DIR)/make/sw/$(tgt_cpu_family)/$(tgt_board)/$(tgt_bsp)
LSCRIPT  += -L$(RTEMS_LIBS)
LSCRIPT  += -L$(bindir)

# Note: The definition of max-page-size must come before the
# specification of the linker script.
ifndef LD_SCRIPT
  LSCRIPT  += -Wl,-z,max-page-size=4096,-T,sysimg.ld,--cref
else
  LSCRIPT  += -Wl,-z,max-page-size=4096,-T,$(LD_SCRIPT)
endif

$(bindir)/rtems.${SHLIBEXTNS}:   LXFLAGS   += -Wl,--no-undefined
$(bindir)/rtems.${SHLIBEXTNS}:   LXFLAGS   += $(LSCRIPT)
$(bindir)/rtems.${SHLIBEXTNS}:   LXFLAGS   += $(LDALLLIBS)
$(bindir)/rtems.${SHLIBEXTNS}:   LXFLAGS   += $(LDWRAPS) -Wl,-soname,$(NAMESPACE):rtems.${SHLIBEXTNS}
$(bindir)/rtems.${SHLIBEXTNS}:   LXFLAGS   += $(GNU_OBJECTS)
$(bindir)/rtems.${SHLIBEXTNS}:   LXFLAGS   += $(CRT_OBJECTS)
$(bindir)/rtems.${SHLIBEXTNS}:   LXFLAGS   += $(RTEMS_START)
$(bindir)/rtems.${SHLIBEXTNS}:   LXFLAGS   += $(RTEMS_ENTRY_FLAG)
$(bindir)/rtems.${SHLIBEXTNS}:   LXFLAGS   += $(RTEMS_LIBRARIES)
# LIBM must be last because of no-whole-archive, so append to internal var
$(bindir)/rtems.${SHLIBEXTNS}:   linkflags_rtems.${SHLIBEXTNS} += $(RTEMS_LIBM)

bin:   $(bins)

clean: $(cleans)

clean_udbi:
	@echo "[RB] {$(tgt_arch)} Removing $(notdir $(bins))"
	$(quiet)rm -f $(bins)

endif
