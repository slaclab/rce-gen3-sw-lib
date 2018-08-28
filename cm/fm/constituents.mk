do_not_document = $(true)

ifeq ($(have_cm),$(true))
#Libraries
libnames := cm_fm
libsrcs_cm_fm := src/FmEplHandler.cc \
                 src/FmEplRegs.cc \
                 src/FmFidTableEntry.cc \
                 src/FmMacTable.cc \
                 src/FmMacTableEntry.cc \
                 src/FmMgrHandler.cc \
		 src/FmConfig.cc \
                 src/FmVlanTableEntry.cc \
                 src/SwmEplRegs.cc \
                 src/SwmFidTableEntry.cc \
                 src/SwmGlobalMibCounters.cc \
                 src/SwmMacTable.cc \
                 src/SwmMacTableD.cc \
                 src/SwmMacTableEntry.cc \
                 src/SwmPacket.cc \
                 src/SwmAllPortMibCounters.cc \
                 src/SwmPortMibCounters.cc \
                 src/SwmPortMscCounters.cc \
                 src/SwmStreamStatus.cc \
                 src/SwmVlanTableEntry.cc \
                 src/SwmPortFwdTable.cc \
                 src/SwmPortVlanConfig.cc \
                 src/SwmTrigger.cc

ifeq ($(have_arm),$(true))
libsrcs_cm_fm += src/arm/FmReg.cc \
		 src/arm/FmTahoe.cc \
		 src/arm/FmTahoeMgr.cc \
		 src/arm/PortMap.cc
endif

else

ifneq ($(findstring i86,$(tgt_arch)),)
libnames := cm_fm
libsrcs_cm_fm := src/SwmFidTableEntry.cc \
                 src/SwmMacTableEntry.cc \
                 src/SwmVlanTableEntry.cc \
                 src/SwmPortMibCounters.cc \
                 src/SwmAllPortMibCounters.cc \
                 src/SwmGlobalMibCounters.cc
endif
endif