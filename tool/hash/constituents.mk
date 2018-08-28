# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the hash package.
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

libnames        := hash
libsrcs_hash    :=
libsrcs_hash    += src/Hash.c

ifeq ($(have_rtems),$(true))
modlibnames     := hash_m
libsrcs_hash_m  := $(libsrcs_hash)
endif

doxy_pkg_c_only=$(true)
