# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the ldr package.
# Author:      S. Maldonado <smaldona@slac.stanford.edu>
# Created:     07/20/13
# Committed:  \$$ by \$Author:$.
# Revision:   \$Revision:$
# Repository: \$HeadURL:$
# Credits:     SLAC
#                              Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

ifeq ($(have_arm),$(true))
ifeq ($(have_rtems),$(true))

libnames        := ldr
libsrcs_ldr     :=
libsrcs_ldr     += src/Ldr.c

doxy_pkg_c_only=$(true)
endif
endif
