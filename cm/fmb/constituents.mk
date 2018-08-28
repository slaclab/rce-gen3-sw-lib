ifeq ($(have_cm),$(true))
#Libraries
libnames := cm_fmb
libsrcs_cm_fmb := src/FmEplRegs.cc \
                 src/FmEgressFidTableEntry.cc \
                 src/FmEgressVlanTableEntry.cc \
                 src/FmIngressFidTableEntry.cc \
                 src/FmIngressVlanTableEntry.cc \
                 src/FmMacTableEntry.cc \
                 src/FmMacTable.cc \
                 src/FmGlortDestEntry.cc \
                 src/FmGlortRamEntry.cc \
                 src/FmMacTcnFifoEntry.cc \
                 src/FmStatsCfg.cc \
                 src/SwmEgressFidTableEntry.cc \
                 src/SwmEgressVlanTableEntry.cc \
                 src/SwmEplRegs.cc \
                 src/SwmPortMibCounters.cc \
                 src/SwmAllPortMibCounters.cc \
                 src/SwmIngressFidTableEntry.cc \
                 src/SwmIngressVlanTableEntry.cc \
                 src/SwmMacTableEntry.cc \
                 src/SwmVlanTableEntry.cc \
                 src/SwmGlortDestEntry.cc \
                 src/SwmGlortRamEntry.cc \
                 src/SwmMacTable.cc \
                 src/SwmMacTableD.cc 

ifeq ($(have_arm),$(true))
libsrcs_cm_fmb += src/arm/FmBali.cc \
		 src/arm/FmBaliMgr.cc
endif

ifeq ($(have_rtems),$(true))
DEFINES += -D__INSIDE_RTEMS_BSD_TCPIP_STACK__
endif

else

ifneq ($(findstring i86,$(tgt_arch)),)
libnames := cm_fmb
libsrcs_cm_fm := src/SwmEgressFidTableEntry.cc \
                 src/SwmEgressVlanTableEntry.cc \
                 src/SwmEplRegs.cc \
                 src/SwmIngressFidTableEntry.cc \
                 src/SwmIngressVlanTableEntry.cc \
                 src/SwmMacTableEntry.cc \
                 src/SwmVlanTableEntry.cc
endif
endif