# Requires OS support.
ifeq ($(have_noose),$(false))

libnames := udp

libsrcs_udp := 
libsrcs_udp += src/Link.cc 

ifeq ($(have_rtems),$(true))

modlibnames := udp

endif


endif
