# Requires OS support.
ifeq ($(have_noose),$(false))

libnames := net

libsrcs_net :=
libsrcs_net += src/Endpoint.cc            
libsrcs_net += src/MacAddress.cc             
libsrcs_net += src/Socket.cc         

docsrcs_net += src/Multicast.cc

ifeq ($(and $(or $(have_rce),$(have_cm)),$(have_cob),$(have_rtems)),$(true))

modlibnames := net

endif

libsrcs_net += src/Interface.cc 
nodocs_net += rtems/Interface.hh
nodocs_net += linux/Interface.hh

endif
