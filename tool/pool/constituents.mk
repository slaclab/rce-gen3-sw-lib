# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the pool package.
# Author:      S. Maldonado <smaldona@slac.stanford.edu>
# Created:     06/16/2013
# Committed:  \$$ by \$Author:$.
# Revision:   \$Revision:$
# Repository: \$HeadURL:$
# Credits:     SLAC
#                              Copyright 2015
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#

ifeq ($(have_arm),$(true))

ifeq ($(have_rtems),$(true))

libnames        := pool
libsrcs_pool    :=

libsrcs_pool         += src/Pool.c

endif

endif
