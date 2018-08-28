# Requires OS support.
ifeq ($(have_noose),$(false))

libnames := dsl_l

libsrcs_dsl_l :=
libsrcs_dsl_l += src/Location.cc
libsrcs_dsl_l += src/Proxy.cc
libsrcs_dsl_l += src/Server.cc
libsrcs_dsl_l += src/Client.cc

ifeq ($(have_linux),$(true))
libsrcs_dsl_l += src/linux/init.cc
libsrcs_dsl_l += src/linux/Server.cc
endif

ifeq ($(have_rtems),$(true))
libsrcs_dsl_l += src/rtems/Server.cc

modnames      := dsl
modlibs_dsl   := service/dsl_l
modlibs_dsl   += service/udp
modlibs_dsl   += service/net
modlibs_dsl   += tool/csi
basetgt_dsl   := platform/rtems
basemod_dsl   := platform/rts
basemod_dsl   += driver/network

tasknames     := dsld
tasksrcs_dsld  := src/rtems/Dsl.cc
basetgt_dsld   := platform/rtems
basemod_dsld   += service/dsl
endif

endif
