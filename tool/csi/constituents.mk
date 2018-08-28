
# comments
 
libnames        := csi
libsrcs_csi     :=
libsrcs_csi     += src/Proxy.cc

ifeq ($(have_rtems),$(true))

modlibnames := csi

endif


