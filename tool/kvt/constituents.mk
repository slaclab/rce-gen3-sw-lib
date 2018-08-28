# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the kvt package.
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


libnames       := kvt
libsrcs_kvt    :=
libsrcs_kvt    += src/Kvt.c

ifeq ($(have_rtems),$(true))
modlibnames    := kvt_m
libsrcs_kvt_m  := $(libsrcs_kvt)
endif

doxy_pkg_c_only=$(true)
