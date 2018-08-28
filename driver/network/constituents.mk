ifeq ($(and $(have_rce),$(have_rtems)),$(true))
ifeq ($(and $(have_arm)),$(true))

libnames := Network

libsrcs_Network :=

libsrcs_Network += src/configure.cc
libsrcs_Network += src/BsdNet.cc
libsrcs_Network += src/Params.cc
libsrcs_Network += src/EthernetConfig.cc
libsrcs_Network += src/armCA9/EthernetConfig.cc

# When this macro is defined the BSD-related headers reveal
# definitions private to the BSD network stack. The name of the macro
# may change in future RTEMS releases but for now (RTEMS 4.9, 4.10)
# this works.  See rtems/rtems_bsdnet.h.
#DEFINES += -D__INSIDE_RTEMS_BSD_TCPIP_STACK__

modnames += network
modlibs_network  := driver/Network 
modslibw_network := $(RTEMS_LIBS)/lib.a
modslibw_network += $(RTEMS_LIBS)/libc.a
modslibw_network += $(RTEMS_LIBS)/libnetworking.a
modslibw_network += $(RTEMS_LIBS)/librpc.a
basetgt_network  := platform/rtems
basemod_network  := platform/rts
endif
endif
