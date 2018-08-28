do_not_document = $(true)

#Libraries
ifeq ($(have_cm),$(true))
libnames := cm_svc cm_print
ifeq ($(have_arm),$(true))
libsrcs_cm_svc := src/ChannelMap.cc \
                  src/CmHw.cc \
                  src/XML.cc \
                  src/ClusterIpInfo.cc \
                  src/PlxBridge.cc
else
libsrcs_cm_svc := src/ChannelMap.cc \
                  src/CmHw.cc \
                  src/opbintctrl.c
endif

ifeq ($(have_linux),$(true))
libnames += cm_syslog
endif

libsrcs_cm_print  := src/linux/Print.cc
libsrcs_cm_syslog := src/linux/SysLog.cc

endif