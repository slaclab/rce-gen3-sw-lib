do_not_document = $(true)

ifeq ($(have_cm),$(true))
#Libraries
libnames := cm_bpdu
libsrcs_cm_bpdu := $(wildcard src/*.cc)

endif