# -*- Mode: makefile; -*-

ifeq ($(have_noose),$(false))
  libnames := dslutil

  rebooter_src := src/RebootBase.cc
  rebooter_src += src/RebootClient.cc
  rebooter_src += src/RebootSpace.cc

  updater_src := src/UpdateBase.cc
  updater_src += src/UpdateClient.cc
  updater_src += src/UpdateListener.cc
  updater_src += src/UpdateSpace.cc

  identifier_src := src/IdBase.cc
  identifier_src += src/IdClient.cc
  identifier_src += src/IdListener.cc
  identifier_src += src/IdSpace.cc

ifeq ($(and $(or $(have_rce),$(have_cm)),$(have_cob)),$(true))
  rebooter_src   += src/reboot_init.cc
  updater_src    += src/update_init.cc
  identifier_src += src/id_init.cc
endif

  libsrcs_dslutil := src/AddressStar.cc
  libsrcs_dslutil += src/Base.cc
  libsrcs_dslutil += $(rebooter_src)
  libsrcs_dslutil += $(updater_src)
  libsrcs_dslutil += $(identifier_src)

ifeq ($(and $(have_rtems),$(have_arm)),$(true))

  libsrcs_dslutil += src/armCA9-rtems/RebootSpace.cc
  libsrcs_dslutil += src/armCA9-rtems/UpdateSpace.cc
  libsrcs_dslutil += src/armCA9-rtems/IdSpace.cc

  basetgt_common := platform/rtems
  basemod_common := platform/rts
  basemod_common += platform/dsl
  basemod_common += platform/atca
  basemod_common += service/shell

  modnames := dsl_reboot
  modsrcs_dsl_reboot := src/armCA9-rtems/reboot_init.cc
  modlibs_dsl_reboot := service/dslutil
  basetgt_dsl_reboot := platform/rtems
  basemod_dsl_reboot := $(basemod_common)

  modnames += dsl_update
  modsrcs_dsl_update := src/armCA9-rtems/update_init.cc
  modlibs_dsl_update := service/dslutil
  basetgt_dsl_update := platform/rtems
  basemod_dsl_update := $(basemod_common)

  modnames += dsl_identifier
  modsrcs_dsl_identifier := src/armCA9-rtems/id_init.cc
  modlibs_dsl_identifier := service/dslutil
  basetgt_dsl_identifier := platform/rtems
  basemod_dsl_identifier := $(basemod_common)

endif # have_rtems&&have_arm

ifeq ($(have_linux),$(true))

  linux_libraries := service/debug service/kvt service/dsl_l service/atca_l
  linux_libraries += tool/hash service/udp tool/concurrency service/net
  linux_libraries += tool/csi tool/memory service/logger tool/exception
  linux_libraries += tool/string tool/io tool/time

  tgtnames :=
  
ifeq ($(have_desktop),$(true))
  libsrcs_dslutil += src/i86-linux/RebootSpace.cc
  libsrcs_dslutil += src/i86-linux/UpdateSpace.cc
  libsrcs_dslutil += src/i86-linux/IdSpace.cc

CPPFLAGS += -I $(FREEIPMI_INCLUDE_DIR)
LXFLAGS += -l freeipmi -L$(FREEIPMI_LIB_DIR)
linux_libraries += service/cob_ipmc
endif # have_desktop

ifeq ($(have_arm),$(true))
  linux_libraries += tool/map tool/mem
  libsrcs_dslutil += src/armCA9-linux/RebootSpace.cc
  libsrcs_dslutil += src/armCA9-linux/UpdateSpace.cc
  libsrcs_dslutil += src/armCA9-linux/IdSpace.cc

endif # have_arm

  tgtnames += dsl_update
ifeq ($(have_arm),$(true))
  tgtsrcs_dsl_update := src/armCA9-linux/dsl_update.cc
endif
ifeq ($(have_desktop),$(true))
  tgtsrcs_dsl_update := src/i86-linux/dsl_update.cc
endif  
  tgtlibs_dsl_update := $(linux_libraries)
  tgtlibs_dsl_update += service/dslutil

  tgtnames += dsl_identify
  tgtsrcs_dsl_identify := src/linux/dsl_identify.cc
  tgtlibs_dsl_identify := $(linux_libraries)
  tgtlibs_dsl_identify += service/dslutil

  tgtnames += dsl_reboot
  tgtsrcs_dsl_reboot := src/linux/dsl_reboot.cc
  tgtlibs_dsl_reboot := $(linux_libraries)
  tgtlibs_dsl_reboot += service/dslutil


endif # have_linux
endif # !have_noose
