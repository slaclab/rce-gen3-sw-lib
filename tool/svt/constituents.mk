# -*-Mode: Makefile;-*-
#
#                               Copyright 2013
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Facility: DAT
# Abstract: Constituents file for the SVT package.
# Author: Stephen Tether <tether@slac.stanford.edu>
# Date created: 2013/12/12
# Last commit: \$Date: 2015-02-23 15:35:41 -0800 (Mon, 23 Feb 2015) $ by \$Author: panetta $.
# Revision number: \$Revision: 4013 $
# Location in repository: \$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/svt/constituents.mk $
# Credits: SLAC


ifeq ($(have_arm),$(true))
ifeq ($(have_rtems),$(true))

libnames        := svt
libsrcs_svt     :=
libsrcs_svt     += src/Svt.c

libnames        += svtstub
libsrcs_svtstub :=
libsrcs_svtstub += src/Svt_Stub.c

doxy_pkg_c_only=$(true)

endif
endif
