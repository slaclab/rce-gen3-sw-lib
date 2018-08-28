# Architecture flags
# ------------------
arch_tgts :=
arch_tgts += i86-linux-32       # Desktop Linux with 32-bit Intel CPU.
arch_tgts += i86-linux-64       # Desktop Linux with 64-bit Intel CPU.

arch_tgts += arm-eabi-rceCA9    # 3rd gen RCE (COB, ARM Cortex-A9) without OS.
arch_tgts += arm-rtems-rceCA9   # Same as above except has RTEMS.
arch_tgts += arm-linux-rceCA9	# Same as above except has Linux.

arch_tgts += arm-eabi-cmCA9     # 3rd gen CM (COB, ARM Cortex-A9) without OS.
arch_tgts += arm-linux-cmCA9	# Same as above except has Linux.

arch_tgts += arm-eabi-zcCA9     # ZC702 (eval board, ARM Cortex-A9) without OS.
arch_tgts += arm-rtems-zcCA9    # Same as above except has RTEMS.

arch_tgts += arm-eabi-zedCA9    # Zedboard (eval board, ARM Cortex-A9) without OS.
arch_tgts += arm-rtems-zedCA9   # Same as above except has RTEMS.

arch_tgts += arm-rtems-zynq     # Same as above but for Zynq-7000.

arch_opts := opt dbg

define arch_opt_template
  arch_tgts += $$(addsuffix -$(1),$$(arch_tgts))
endef
$(foreach opt,$(arch_opts),$(eval $(call arch_opt_template,$(opt))))



# Our build system will sometimes invoke a sub-make without defining
# tgt_arch. The sub-makes use the project and package makefiles which
# unconditionally include this file, which tries to parse tgt_arch.
ifneq ($(tgt_arch),)



# Separate the components of tgt_arch using the dash as a separator.
archwords      := $(subst -, ,$(strip $(tgt_arch)))
tgt_opt        := $(filter $(arch_opts),$(archwords))
archwords      := $(filter-out $(arch_opts),$(archwords))
tgt_cpu_family := $(word 1,$(archwords))
tgt_os         := $(word 2,$(archwords))
tgt_board      := $(word 3,$(archwords))
archstring     := $(tgt_cpu_family)-$(tgt_os)$(if $(tgt_board),-$(tgt_board))

# Further info:
# tgt_brd - More general than tgt_board, e.g., its value is "rce" for all RCE boards, etc.
# tgt_bsp - If applicable, the RTEMS BSP name.
# tgt_cpu - If applicable, the value to pass to the GCC option -mcpu.
# tgt_gen - If applicable, the generation designator for RCE or CM board.
# tgt_flt - If applicable, the floating-point support options to pass to GCC.
# tgt_mpu - The model number to append to the CPU family to make the specific CPU name.
tgt_brd := $(tgt_cpu_family)
tgt_bsp :=
tgt_cpu := $(tgt_cpu_family)
tgt_gen :=
tgt_flt :=
tgt_mpu := $(tgt_board)


# If board information is missing for a platform we assume that we
# only care about the general CPU architecture.
ifeq ($(tgt_board),)
  tgt_board := $(tgt_cpu_family)
endif


# Third generation (COB) RCEs with Zynqs.
# For now cmCA9 and rceCA9 will use the zedboard BSP even though
# it declares half the memory that the DTM/DPM actually has.
ifeq ($(tgt_board),rceCA9)
  tgt_brd := rce
  tgt_cpu := cortex-a9
  tgt_mpu := CA9
  tgt_gen := gen3
  tgt_flt := -march=armv7-a -mtune=cortex-a9
ifeq ($(tgt_os),rtems)
  tgt_bsp := xilinx_zynq_ml1
  tgt_flt += -mfpu=neon -mfloat-abi=softfp 
endif
ifeq ($(tgt_os),linux)
  tgt_flt += -mfpu=neon -mfloat-abi=hard 
endif
endif

# Third generation (COB) CMs with Zynqs
# For now cmCA9 and rceCA9 will use the zedboard BSP even though
# it declares half the memory that the DTM/DPM actually has.
ifeq ($(tgt_board),cmCA9)
  tgt_brd := cm
  tgt_cpu := cortex-a9
  tgt_mpu := CA9
  tgt_gen := gen3
ifeq ($(tgt_os),rtems)
  tgt_bsp := xilinx_zynq_ml1
  tgt_flt += -mfpu=neon -mfloat-abi=softfp 
endif
ifeq ($(tgt_os),linux)
  tgt_flt += -mfpu=neon -mfloat-abi=hard 
endif
endif

# ZC702 (eval board) with Zynq
ifeq ($(tgt_board),zcCA9)
  tgt_brd := zc
  tgt_cpu := cortex-a9
  tgt_mpu := CA9
  tgt_gen := gen3
ifeq ($(tgt_os),rtems)
  tgt_bsp := xilinx_zynq_zc702
  tgt_flt += -mfpu=neon -mfloat-abi=softfp 
endif
ifeq ($(tgt_os),linux)
  tgt_flt += -mfpu=neon -mfloat-abi=hard 
endif
endif

# Zedboard (eval board) with Zynq
ifeq ($(tgt_board),zedCA9)
  tgt_brd := zed
  tgt_cpu := cortex-a9
  tgt_mpu := CA9
  tgt_gen := gen3
ifeq ($(tgt_os),rtems)
  tgt_bsp := xilinx_zynq_zedboard
  tgt_flt += -mfpu=neon -mfloat-abi=softfp 
endif
ifeq ($(tgt_os),linux)
  tgt_flt += -mfpu=neon -mfloat-abi=hard 
endif
endif

tgt_cpu_model := $(tgt_cpu_family)$(tgt_mpu)

# Initialize the feature test macros and their auxiliaries.
false :=# Considered false by $(if, and, or )
true  := t# Anything non-null is considered true.

have_std_cpp   := $(false)# C++ with integer math and standard libs.
have_std_c     := $(false)# C with integer math and standard libs.
have_network   := $(false)# BSD sockets.
have_threads   := $(false)# DAT generic threading API.
have_float     := $(false)# Add floating math to std_cpp and std_c.
have_posix     := $(false)# POSIX APIs including pthreads.
have_linux     := $(false)# Full-blown Linux.
have_rtems     := $(false)# Classic RTEMS API.
have_noose     := $(false)# No OS Environment.
have_ipmc_c    := $(false)# IPMC with C and a limited library.
have_superuser := $(false)# Programs normally or always run in supervisor mode.
have_desktop   := $(false)# Desktop system, not an RCE or other embedded system.
have_64bit     := $(false)# Desktop system with a 64-bit C/C++ programming model, i.e.,
                          # int is still 32-bit but pointers are 64-bit.

have_arm       := $(false)# ARM CPU user-mode instruction set.
have_thumb     := $(false)# ARM Thumb2 compressed instructions.
have_armCA9    := $(false)# ARM Cortex-A9 full instruction set..
have_i86       := $(false)# Intel i86-compatible user-mode instruction set.
have_sparc     := $(false)# SPARC user-mode instruction set.
have_apu       := $(false)# Auxiliary processing unit(s) available.

have_mmu       := $(false)# Memory Management Unit available.
have_address_translation := $(false)# MMU is being used to translate addresses.

have_zynq      := $(false)# Xilinx Zynq 7000 features.
have_cm        := $(false)# Xilinx FPGA with CM firmware.
have_rce       := $(false)# Xilinx FPGA with RCE firmware.
have_pgp       := $(false)# PGP protocol ports.
have_ethernet  := $(false)# Raw ethernet.
have_cob       := $(false)# System is part of a Cluster-On-a-Board.
have_i2c       := $(false)# Hardware interrogation via I2C is available.
have_config_flash := $(false)# RCE configuration flash (soldered on).
have_config_sd := $(false)# SD module config memory.

ifeq ($(archstring),i86-linux-32)
have_std_cpp := $(true)
have_std_c   := $(true)
have_network := $(true)
have_threads := $(true)
have_float   := $(true)
have_posix   := $(true)
have_i86     := $(true)
have_desktop := $(true)
have_linux   := $(true)
have_mmu     := $(true)
have_address_translation := $(true)
endif

ifeq ($(archstring),i86-linux-64)
have_std_cpp := $(true)
have_std_c   := $(true)
have_network := $(true)
have_threads := $(true)
have_float   := $(true)
have_posix   := $(true)
have_i86     := $(true)
have_64bit   := $(true)
have_desktop := $(true)
have_linux   := $(true)
have_mmu     := $(true)
have_address_translation := $(true)
endif

ifeq ($(archstring),arm-eabi-rceCA9)
have_float     := $(true)
have_noose     := $(true)
have_superuser := $(true)
have_arm       := $(true)
have_armCA9    := $(true)
have_zynq      := $(true)
have_rce       := $(true)
have_pgp       := $(true)
have_ethernet  := $(true)
have_cob       := $(true)
have_i2c       := $(true)
have_config_sd := $(true)
endif

ifeq ($(archstring),arm-eabi-cmCA9)
have_float     := $(true)
have_noose     := $(true)
have_superuser := $(true)
have_arm       := $(true)
have_armCA9    := $(true)
have_zynq      := $(true)
have_cm        := $(true)
have_pgp       := $(true)
have_ethernet  := $(true)
have_cob       := $(true)
have_i2c       := $(true)
have_config_sd := $(true)
have_mmu       := $(true)
endif

ifeq ($(archstring),arm-eabi-zcCA9)
have_float     := $(true)
have_noose     := $(true)
have_superuser := $(true)
have_arm       := $(true)
have_armCA9    := $(true)
have_zynq      := $(true)
have_ethernet  := $(true)
have_i2c       := $(true)
have_config_sd := $(true)
have_mmu       := $(true)
endif

ifeq ($(archstring),arm-eabi-zedCA9)
have_float     := $(true)
have_noose     := $(true)
have_superuser := $(true)
have_arm       := $(true)
have_armCA9    := $(true)
have_zynq      := $(true)
have_ethernet  := $(true)
have_i2c       := $(true)
have_config_sd := $(true)
have_mmu       := $(true)
endif

ifeq ($(archstring),arm-rtems-rceCA9)
have_std_cpp   := $(true)
have_std_c     := $(true)
have_float     := $(true)
have_network   := $(true)
have_threads   := $(true)
have_rtems     := $(true)
have_superuser := $(true)
have_arm       := $(true)
have_armCA9    := $(true)
have_zynq      := $(true)
have_rce       := $(true)
have_pgp       := $(true)
have_ethernet  := $(true)
have_cob       := $(true)
have_i2c       := $(true)
have_config_sd := $(true)
have_mmu       := $(true)
endif

ifeq ($(archstring),arm-rtems-zynq)
have_std_cpp   := $(true)
have_std_c     := $(true)
have_float     := $(true)
have_rtems     := $(true)
have_superuser := $(true)
have_arm       := $(true)
have_armCA9    := $(true)
have_zynq      := $(true)
have_mmu       := $(true)
endif

ifeq ($(archstring),arm-rtems-zcCA9)
have_std_cpp   := $(true)
have_std_c     := $(true)
have_float     := $(true)
have_network   := $(true)
have_threads   := $(true)
have_rtems     := $(true)
have_superuser := $(true)
have_arm       := $(true)
have_armCA9    := $(true)
have_zynq      := $(true)
have_ethernet  := $(true)
have_i2c       := $(true)
have_mmu       := $(true)
endif

ifeq ($(archstring),arm-rtems-zedCA9)
have_std_cpp   := $(true)
have_std_c     := $(true)
have_float     := $(true)
have_network   := $(true)
have_threads   := $(true)
have_rtems     := $(true)
have_superuser := $(true)
have_arm       := $(true)
have_armCA9    := $(true)
have_zynq      := $(true)
have_ethernet  := $(true)
have_i2c       := $(true)
have_mmu       := $(true)
endif

ifeq ($(archstring),arm-linux-rceCA9)
have_std_cpp   := $(true)
have_std_c     := $(true)
have_float     := $(true)
have_network   := $(true)
have_threads   := $(true)
have_linux     := $(true)
have_arm       := $(true)
have_armCA9    := $(true)
have_zynq      := $(true)
have_rce       := $(true)
have_pgp       := $(true)
have_ethernet  := $(true)
have_cob       := $(true)
have_i2c       := $(true)
have_config_sd := $(true)
have_mmu       := $(true)
have_address_translation := $(true)
endif

ifeq ($(archstring),arm-linux-cmCA9)
have_std_cpp   := $(true)
have_std_c     := $(true)
have_float     := $(true)
have_network   := $(true)
have_threads   := $(true)
have_linux     := $(true)
have_arm       := $(true)
have_armCA9    := $(true)
have_zynq      := $(true)
have_cm        := $(true)
have_ethernet  := $(true)
have_cob       := $(true)
have_i2c       := $(true)
have_config_sd := $(true)
have_mmu       := $(true)
have_address_translation := $(true)
endif

# Make sure certain definitions exist
ifeq ($(tgt_brd),)
$(error Build system bug: 'tgt_brd' is undefined)
endif

ifeq ($(tgt_cpu_family),)
$(error Build system bug: 'tgt_cpu_family' is undefined)
endif

ifeq ($(tgt_os),)
$(error Build system bug: 'tgt_os' is undefined)
endif

# We assume that libraries, modules and applications are built on
# Intel Linux desktop machines, using cross-development tools when the
# target platform is not Intel Linux. One problem with this is that
# the environment variable LD_LIBRARY_PATH has an undocumented side
# effect on GNU ld; it overrides link-time search paths specified by
# other means. LD_LIBRARY_PATH, if set at all, needs to be set to
# allow the build tools to work but is not relevant for target
# libraries. Hence we use "env" to unset LD_LIBRARY_PATH temporarily
# when cross-linking.
XLINK_ENV := /bin/env -u LD_LIBRARY_PATH

# The tools we use:
# AS  - assembler
# CPP - C/C++ preprocessor
# CC  - C Compiler
# CXX - C++ compiler
# LD  - Static linker used to create libraries. For RTEMS targets
#       we don't create true libraries, just amalgamated object
#       files meant for inclusion in the RTEMS "application".
#       For Linux we create true shared libraries.
# LX  - Static linker used to create applications. For RTEMS
#       an application is RTEMS itself + user init tasks +
#       "libraries" + system startup code. For Linux it's an executable
#       program with a main() function.
# ----- For RTEMS relocatable modules only -----
# The code is compiled and linked so that entities not explicitly
# marked with the EXPORT macro in source code are not visible to
# the RTEMS dynamic linker.
# MCXX - C++ compiler
# MLD  - Static linker, "library" amalgamation.
# MLX  - Static linker, module.

# ARM Xilinx Linux specific targets
ifeq ($(tgt_cpu_family)-$(tgt_os),arm-linux)
  CROSS_COMPILE ?= arm-linux-gnueabihf-	
  AS  	   := $(CROSS_COMPILE)as
  CPP 	   := $(CROSS_COMPILE)gcc -E
  CC  	   := $(CROSS_COMPILE)gcc
  CXX 	   := $(CROSS_COMPILE)g++
  LD  	   := $(XLINK_ENV) $(CROSS_COMPILE)g++
  LX  	   := $(XLINK_ENV) $(CROSS_COMPILE)g++
  OBJCOPY  := $(CROSS_COMPILE)objcopy
  OBJDUMP  := $(CROSS_COMPILE)objdump
  STRIP    := $(CROSS_COMPILE)strip

  LIBEXTNS  = so
  DEPFLAGS  = -MM
  DEFINES  += -D_REENTRANT -DEXPORT=
  CPPFLAGS  = -I$(PTH_INCLUDE_DIR)
  CFLAGS    = -Wno-psabi $(tgt_flt) -mcpu=$(tgt_cpu) -fPIC -Wall
  CXXFLAGS  = $(CFLAGS)
  CASFLAGS  = -x assembler-with-cpp
  LDFLAGS   = -shared -Wl,-soname,lib$*.$(LIBEXTNS)
  # The Pth library for cross-linking is not, can't be, a system
  # library so we have to tell the linker where to find it for
  # applications that may need it. At run time we assume it's
  # installed as a target system library. -rpath-link doesn't
  # modify the executable so those that don't use Pth don't
  # know what they're missing.
ifneq ($(PTH_VERSION),)
  PTH_INCLUDE_DIR := $(PTH_ROOT)/v$(PTH_VERSION)-$(tgt_cpu_model)/include
  PTH_LIB_DIR     := $(PTH_ROOT)/v$(PTH_VERSION)-$(tgt_cpu_model)/lib
else
  PTH_INCLUDE_DIR := $(PTH_ROOT)/include
  PTH_LIB_DIR     := $(PTH_ROOT)/lib
endif
  PTH_LIBSLIB     := $(PTH_LIB_DIR)/pth
  LXFLAGS   = -lpthread -lrt -ldl -Wl,-rpath-link,$(PTH_LIB_DIR)
endif


# i86 Linux desktops - use native tools (target platform == host platform).
ifeq ($(have_i86),$(true))
ifeq ($(have_linux),$(true))
  AS  	   := as
  CPP 	   := gcc -E
  CC  	   := gcc
  CXX 	   := g++
  LD  	   := g++
  LX  	   := g++
  OBJCOPY  := objcopy
  OBJDUMP  := objdump
  STRIP    := strip

ifeq ($(have_64bit),$(true))
  MFLAG = -m64
else
  MFLAG = -m32
endif

  LIBEXTNS  = so
  DEPFLAGS  = -MM
  DEFINES  +=-D_REENTRANT -D__pentium__ -D__STDC_FORMAT_MACROS
  DEFINES  += -DEXPORT=__attribute__\(\(visibility\(\"default\"\)\)\)
  CPPFLAGS  =
  CFLAGS    = $(MFLAG) -fPIC -Wall
  CXXFLAGS  = $(CFLAGS)
  CASFLAGS  = -x assembler-with-cpp
  LDFLAGS   = $(MFLAG) -shared 
  # We assume that Pth is installed as a host system library so that
  # it will will be found both at link time and at run time. No need
  # for -rpath-link or -rpath. The use of PTH_LIBSLIB in a consitiuents.mk
  # file will create a NEEDED entry in the library being built, which is
  # all we need.
  PTH_INCLUDE_DIR :=
ifeq ($(have_64bit),$(true))
  PTH_LIB_DIR     := /usr/lib64
else
  PTH_LIB_DIR     := /usr/lib
endif
  PTH_LIBSLIB     := $(PTH_LIB_DIR)/pth
  # Freeipmi is not a system library, thus it needs a pair of paths.
  FREEIPMI_INCLUDE_DIR := $(FREEIPMI_ROOT)-$(tgt_cpu_model)/include
  FREEIPMI_LIB_DIR     := $(FREEIPMI_ROOT)-$(tgt_cpu_model)/lib
  LXFLAGS   = $(MFLAG) -lrt -ldl -Wl,--export-dynamic
endif
endif


# RTEMS specific flags
ifeq ($(tgt_os),rtems)

# ARM specific flags
ifeq ($(tgt_cpu_family),arm)
  tool_suite := arm-rtems
  MKIMAGE  := $(UBOOT_ROOT)/tools/mkimage  
endif

  rtems_vers  := $(subst ., ,$(call reverse,$(subst /, ,$(RTEMS_ROOT))))
  tgt_os_maj  := $(strip $(word 1,$(rtems_vers)))
  tgt_os_min  := $(strip $(word 2,$(rtems_vers)))
  tgt_os_rev  := $(strip $(word 3,$(rtems_vers)))
  tgt_os_rc   := $(strip $(word 4,$(rtems_vers)))

  tgt_os_vers := $(tgt_os_maj).$(tgt_os_min)

  AS  	  := $(tgt_os)-gasm
  CPP 	  := $(tool_suite)$(tgt_os_vers)-cpp
  CC  	  := $(tgt_os)-gcc
  CXX 	  := $(tgt_os)-g++
  # LD is only used to generate internal .a files
  LD  	  := $(tool_suite)$(tgt_os_vers)-ld
  LX  	  := $(tool_suite)$(tgt_os_vers)-g++
  MLX  	  := $(tgt_os)-ld
  OBJCOPY := $(tool_suite)$(tgt_os_vers)-objcopy
  OBJDUMP := $(tool_suite)$(tgt_os_vers)-objdump
  STRIP   := $(tool_suite)$(tgt_os_vers)-strip

  LIBEXTNS  = a
  SHLIBEXTNS = so
  SVTEXTNS  = svt
  TASKEXTNS = exe
  NAMESPACE = system
  DEPFLAGS  = -B$(RTEMS_LIBS) -MM
  DEFINES   += -D__DYNAMIC_REENT__
  CPPFLAGS  = 
  CFLAGS    =
  CXXFLAGS  = $(CFLAGS) -std=gnu++11
  CASFLAGS  = 

  LDFLAGS   = -r
  LXFLAGS   = -shared -nostdlib
  LXFLAGS  += -Wl,--hash-style=gnu

  MDEFINES  = $(DEFINES)
  MDEFINES += -UEXPORT -DEXPORT='__attribute__((visibility("default")))'

  MLXFLAGS := -Wl,--no-undefined
  MLXFLAGS += -Wl,--allow-shlib-undefined
  MLXFLAGS += -Wl,--unresolved-symbols=ignore-in-shared-libs
endif


ifeq ($(tgt_os),eabi)

# Zynq FPGA specific targets (no OS and no modules)
ifeq ($(tgt_cpu_family),arm)
  CROSS_COMPILE ?=arm-xilinx-eabi-
  tool_suite := $(CROSS_COMPILE)

  BOOTGEN  := bootgen

  # Additional flags used by Xilinx:
  CFLAGS   += -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)"
  CFLAGS   += -fPIC -Wno-psabi
endif

  AS  	   := $(tool_suite)as
  CPP 	   := $(tool_suite)cpp
  CC  	   := $(tool_suite)gcc
  CXX 	   := $(tool_suite)g++
  LD  	   := $(tool_suite)ld
  LX  	   := $(tool_suite)g++
  OBJCOPY  := $(tool_suite)objcopy
  OBJDUMP  := $(tool_suite)objdump
  SIZE     := $(tool_suite)size
  STRIP    := $(tool_suite)strip

  LIBEXTNS  = a
  DEPFLAGS  = -MM
  DEFINES  += -DEXPORT=
  CPPFLAGS  =
  CFLAGS   += -mcpu=$(tgt_cpu) $(tgt_flt) -Wall
  CXXFLAGS  = $(CFLAGS)
  CASFLAGS  = -x assembler-with-cpp
  LDFLAGS   = -r
  LXFLAGS   = -fno-builtin -nostartfiles -nodefaultlibs -mcpu=$(tgt_cpu) $(tgt_flt)
endif


ifeq ($(tgt_opt),dbg)
  CFLAGS  += -g
endif

ifeq ($(tgt_opt),dbgopt)
  CFLAGS  += -O2 -g
endif

ifeq ($(tgt_opt),opt)
  CFLAGS  += -O2
endif

endif
