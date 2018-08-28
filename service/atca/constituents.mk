# Requires OS support.
ifeq ($(have_noose),$(false))

libnames     := atca_l

libsrcs_atca_l :=
libsrcs_atca_l += src/Address.cc
libsrcs_atca_l += src/Client.cc
libsrcs_atca_l += src/Space.cc
libsrcs_atca_l += src/Attributes.cc

ifeq ($(and $(or $(have_rce),$(have_cm)),$(have_cob)),$(true))

libsrcs_atca_l += src/init.cc

ifeq ($(have_rtems),$(true))

modnames     := atca
modlibs_atca := service/atca_l
basetgt_atca := platform/rtems
basemod_atca := driver/network
basemod_atca += platform/rts
basemod_atca += service/dsl

endif

endif

endif
