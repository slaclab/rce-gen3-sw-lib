# A consituents.mk file tells what binaries are built in a package,
# what source files must be compiled and to make them and what libraries
# are must be searchMeta-variables in the descriptions below are
# enclosed in <>.

# Lists of package binaries. Try to make these the same for all
# platforms where possible.

# <bintype>names := <name1> <name2> ...

# where <bintype> can be one of
#   tgt     An executable loaded by the bootstrap loader.
#           Normally contains the OS + application startup + core application libraries.
#   mod     A dynamically loaded module.
#   lib     A library used to build tgt-type executables.
# Libs and modules are compiled with different compiler options. You
# can't have both a lib and a module with the same name. 

# For each binary built there needs to be a list of source code files
# for application binaries. One list for each package binary. Each
# list must start off like this:

# <srctype>srcs_<name1> :=

# where <srctype> is determined by the <bintype> of <name1>:
#     <bintype>    <srctype>
#       tgt           tgt
#       mod           mod
#       lib           lib

# The list of source files for each binary will in general have a part
# that is generic and parts that depend on aspects of the platform
# such as the OS, the CPU architecture and the supporting chipset. As
# shown above the list variable should be defined as null using
# ":=". You then add to the list using assignments with +=. Some of
# these assignments will be inside conditionals testing aspects of the
# platform.

# <scrtype>srcs_<name1> += src/<file1>.cc
# <srctype>srcs_<name1> += src/<file2>.cc
# <srctype>srcs_<name1> += src/<file3>.cc
#
# ifeq($(tgt_os),linux)
# <scrtype>srcs_<name1> += src/linux/<file4>.cc
# endif
#
# ifeq($(have_config_flash),$(true))
# <srctype>srcs_<name1> += src/<file5>.cc
# <scrtype>srcs_<name1> += src/<file6>.cc
# endif

# There are a number of standard variables you can use to test various
# aspects of the platform:
#     tgt_cpu_family   The general architecture, e.g., i86, arm.
#     tgt_os           The OS, e.g., linux, rtems.
#     tgt_board        The kind of board on which the system is mounted.
# There are special subdirectories of src/ or test/ related to these
# settings. The subdirectory names may have the forms <os>, <board>,
# <cpu> or <cpu>-<os>, i.e., # linux/, arm-rtems/. These
# are where you are expected to place headers and implementation
# source code supporting these major divisions.

# There are also a number of feature-test variables which you can use
# to test for the presence or absence of a specific feature. These
# variables all have names beginning with "have_". See
# make/sw/flags.mk to see all the variables of this type
# available. These variables are not related to specific
# subdirectories since there too many of them. Some of them do overlap
# the "tgt" variables, e.g., have_linux, have_arm,
# have_armCA9.

# For each binary built you must list the system libraries needed,
# if any.  Note that <lib> is the name of the library, not of the
# file: i.e.  <lib> for 'libc.so' is 'c'. System libraries are needed
# mostly for building binaries for Linux.

# <bintype>slib_<name1> := <dir1>/<lib1> <dir2>/<lib2> ...

# For each binary built you must list the libraries needed, if any,
# belonging to this or other DAT projects.  Note that <lib> is the
# name of the library, not of the file: i.e.  <lib> for 'libc.so' is
# 'c'.

# <bintype>libs_<name1> := <DAT project1>/<lib1> <DAT project 2>/<lib2> ...

# For each binary built you must list any special #include directories
# needed.  Note that the top level workspace/ directory is already in
# the search path. This list is pretty much obsolete now, replaced by
# using datCode.hh in source files.

# <bintype>incs_<name1> := <DAT project 1>/<incdir1> <DAT project 2>/<incdir2>

# For each binary built that needs to search system include
# directories you must list those directories.

# <bintype>sinc_<name1> := <dir1> <dir2> ...

# For each module built you must specify a major version number, minor
# version number, and the name of a branch of development. Normally
# the latter will be "main" but may be changed to indicate that source
# code is coming from repository branch other than the trunk. This
# information will become part of the module filename, e.g.,
# foo.1.0.main.so.

# majorv_<modname> := 1
# minorv_<modname> := 0
# branch_<modname> := main

# ---------------------------------
# Example. Project Ay, package foo builds a module foo.1.0.main.so which needs
# libraries from projects Bee and Cee. Package foo also builds a
# library bar which it uses in its module but which other packages or
# projects may use to build their modules.

#modnames := foo
#
#modsrcs_foo :=
#
#ifeq ($(have_std_cpp),$(true))
## Generic C++ code.
#modsrcs_foo += src/alpha.cc
#modsrcs_foo += src/beta.cc
#endif
#
#majorv_foo := 1
#minorv_foo := 0
#branch_foo : = main
#
#libs_foo := Ay/bar Bee/bazz Cee/frobozz
#
#
#libnames := bar
#
#libsrcs_bar :=
#ifeq ($(have_std_cpp),$(true))
#ifeq ($(have_posix),$(true))
#libsrcs_bar += src/posix_gamma.cc
#else ifeq ($(have_rtems),$(true))
#libsrcs_bar += src/rtems/gamma.cc
#endif
#endif
