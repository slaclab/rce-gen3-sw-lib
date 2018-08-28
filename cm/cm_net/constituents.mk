do_not_document = $(true)

ifeq ($(have_cm),$(true))
#Libraries
libnames := cm_net
libsrcs_cm_net := src/Service.cc \
                  src/PauseHandler.cc \
                  src/DLFHandler.cc \
                  src/IGMPv3Handler.cc

endif
