# -*-Mode: Makefile;-*-
#
# Facility:    DAT
# File:        constituents.mk
# Abstract:    Constituents file for the PGP package.
# Author:      S. Maldonado <smaldona@slac.stanford.edu>
# Created:     08/20/14
# Committed:  \$$ by \$Author:$.
# Revision:   \$Revision:$
# Credits:     SLAC
#                              Copyright 2015
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
ifeq ($(have_linux),$(true))
ifeq ($(have_desktop),$(true))
ifeq ($(have_64bit),$(true))
tgtnames :=
tgtnames += bdiemsrv
tgtsrcs_bdiemsrv := test/BdiEm_Server.c

tgtnames += bdiemcli
tgtsrcs_bdiemcli := test/BdiEm_Client.c

endif
endif
endif

ifeq ($(have_arm),$(true))
ifeq ($(have_rtems),$(true))

modnames           := pgp
modsrcs_pgp        := src/Pgp.c src/Pgp_Access.c src/Pgp_Dump.c
modlibs_pgp        := task/taskstub
basetgt_pgp        := platform/rtems
basemod_pgp        := service/shell
basemod_pgp        += service/sas

modnames           += rri
modsrcs_rri        := src/Rri.c
basetgt_rri        := platform/rtems
basemod_rri        := service/shell
basemod_rri        += service/sas
basemod_rri        += ppi/pgp

modnames           += rriEm
modsrcs_rriEm      := src/RriEm.c     
basetgt_rriEm      := platform/rtems  
basemod_rriEm      := service/shell   
basemod_rriEm      += service/sas     
basemod_rriEm      += ppi/pgp

modnames           += bdi
modsrcs_bdi        := src/Bdi.c
basetgt_bdi        := platform/rtems
basemod_bdi        := service/shell
basemod_bdi        += service/sas
basemod_bdi        += ppi/pgp

modnames           += bdiEm
modsrcs_bdiEm      := src/BdiEm.c     
basetgt_bdiEm      := platform/rtems  
basemod_bdiEm      := service/shell   
basemod_bdiEm      += service/sas     
basemod_bdiEm      += ppi/pgp         

modnames           += rri_test
modsrcs_rri_test   := test/Rri_Test.c
basetgt_rri_test   := platform/rtems
basemod_rri_test   := service/shell
basemod_rri_test   += ppi/pgp
basemod_rri_test   += ppi/rri

modnames           += rriEm_test
modsrcs_rriEm_test := test/RriEm_Test.c
basetgt_rriEm_test := platform/rtems
basemod_rriEm_test := service/shell
basemod_rriEm_test += ppi/rriEm
basemod_rriEm_test += ppi/bdiEm_test

modnames           += bdi_test
modsrcs_bdi_test   := test/Bdi_Test.c
basetgt_bdi_test   := platform/rtems
basemod_bdi_test   := service/shell
basemod_bdi_test   += ppi/pgp
basemod_bdi_test   += ppi/bdi

modnames           += bdiEm_test
modsrcs_bdiEm_test := test/BdiEm_Test.c
basetgt_bdiEm_test := platform/rtems
basemod_bdiEm_test := service/shell
basemod_bdiEm_test += driver/network
basemod_bdiEm_test += ppi/bdiEm ppi/xaui service/sas

endif
endif
