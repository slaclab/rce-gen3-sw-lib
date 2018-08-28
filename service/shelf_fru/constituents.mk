# -*-Mode: Makefile;-*-
#                               Copyright 2011
#                                    by
#                       The Board of Trustees of the
#                    Leland Stanford Junior University.
#                           All rights reserved.
#
# Abstract: constituents.mk for service/shelf_fru


libnames  :=


ifeq ($(have_desktop),$(true))
CPPFLAGS +=  -I $(FREEIPMI_INCLUDE_DIR)
LXFLAGS += -l freeipmi -L$(FREEIPMI_LIB_DIR)

libnames += shelf_fru
libsrcs_shelf_fru := src/shelf_ip_info.c

tgtnames += set_shelf_ip_info
tgtlibs_set_shelf_ip_info := service/shelf_fru
tgtsrcs_set_shelf_ip_info := src/set_shelf_ip_info.c

tgtnames += delete_shelf_ip_info
tgtlibs_delete_shelf_ip_info := service/shelf_fru
tgtsrcs_delete_shelf_ip_info := src/delete_shelf_ip_info.c

tgtnames += display_shelf_ip_info
tgtlibs_display_shelf_ip_info := service/shelf_fru
tgtsrcs_display_shelf_ip_info := src/display_shelf_ip_info.c

endif