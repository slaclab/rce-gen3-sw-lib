# -*-Mode: Makefile;-*-
#                               Copyright 2017
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Abstract:    constituents.mk for configuration/svtcfg
# Author:      S. Maldonado  ---  smaldona@slac.stanford.edu
# Created:     2014/01/18
# Credits:     SLAC

ifeq ($(tgt_os),rtems)

svtnames := app sys sas rriEm

svtsrcs_app := src/app.c
basetgt_app := platform/rtems
namespace_app := config

svtsrcs_sys := src/sys.c
basetgt_sys := platform/rtems
namespace_sys := config

svtsrcs_sas := src/sas.c             
basetgt_sas := platform/rtems        
namespace_sas := config

svtsrcs_rriEm := src/rriEm.c
basetgt_rriEm := platform/rtems
namespace_rriEm := config

endif
