# -*-Mode: Makefile;-*-
#                               Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Abstract: constituents.mk for service/cob_ipmc

libnames :=
tgtnames :=

ifeq ($(have_desktop),$(true))
CPPFLAGS += -I $(FREEIPMI_INCLUDE_DIR)
LXFLAGS += -l freeipmi -L$(FREEIPMI_LIB_DIR)

libnames += cob_ipmc
libsrcs_cob_ipmc := src/COB_cmds.c
libsrcs_cob_ipmc += src/COB_util.c
libsrcs_cob_ipmc += src/COB_ipmc.c

tgtnames += cob_rce_reset
tgtlibs_cob_rce_reset = service/cob_ipmc
tgtsrcs_cob_rce_reset = src/cob_rce_reset.c
tgtnames += cob_refresh_shelf_info
tgtlibs_cob_refresh_shelf_info = service/cob_ipmc
tgtsrcs_cob_refresh_shelf_info = src/cob_refresh_shelf_info.c
tgtnames += cob_rtm_init
tgtlibs_cob_rtm_init = service/cob_ipmc
tgtsrcs_cob_rtm_init = src/cob_rtm_init.c
tgtnames += cob_fru_init
tgtlibs_cob_fru_init = service/cob_ipmc
tgtsrcs_cob_fru_init = src/cob_fru_init.c
tgtnames += cob_raw_cmd
tgtlibs_cob_raw_cmd = service/cob_ipmc
tgtsrcs_cob_raw_cmd = src/cob_raw_cmd.c
tgtnames += cob_rmb_init
tgtlibs_cob_rmb_init = service/cob_ipmc
tgtsrcs_cob_rmb_init = src/cob_rmb_init.c
tgtnames += cob_cold_data_reset
tgtlibs_cob_cold_data_reset = service/cob_ipmc
tgtsrcs_cob_cold_data_reset = src/cob_cold_data_reset.c
tgtnames += cob_rce_set_zombie
tgtlibs_cob_rce_set_zombie = service/cob_ipmc
tgtsrcs_cob_rce_set_zombie = src/cob_rce_set_zombie.c
tgtnames += cob_rce_clr_zombie
tgtlibs_cob_rce_clr_zombie = service/cob_ipmc
tgtsrcs_cob_rce_clr_zombie = src/cob_rce_clr_zombie.c
tgtnames += cob_dump
tgtlibs_cob_dump = service/cob_ipmc
tgtsrcs_cob_dump = src/COB_dump.c src/cob_dump.c
tgtnames += cob_dump_bsi
tgtlibs_cob_dump_bsi = service/cob_ipmc
tgtsrcs_cob_dump_bsi = src/COB_dump_bsi.c src/cob_dump_bsi.c


endif