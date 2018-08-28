ifeq ($(have_cm),$(true))

ifeq ($(tgt_gen),gen3)
libnames := cm_cntrl
tgtnames := fmd bsi_dump
endif

ifeq ($(tgt_gen),gen2)
libnames := cm_control cm_dtm
tgtnames    := dtm
tgtnames    += cim
tgtnames    += brdinit
tgtnames    += iic
endif

ifeq ($(tgt_gen),gen1)
libnames := cm_control
tgtnames    += cim
tgtnames    += brdinit
endif

libsrcs_cm_cntrl := src/SwmDebug.cc
libsrcs_cm_cntrl += src/SwmRemoteMgr.cc
libsrcs_cm_cntrl += src/Manager.cc
libsrcs_cm_cntrl += src/ManagerB.cc

libsrcs_cm_control := src/SwmDebug.cc
libsrcs_cm_control += src/SwmRemoteMgr.cc
libsrcs_cm_control += src/CimConsoleTask.cc
libsrcs_cm_control += src/RceSlaveTest.cc
libsrcs_cm_control += src/SocketTest.cc
libsrcs_cm_control += src/CmFwUploadTask.cc
libsrcs_cm_control += src/DisplayTask.cc
libsrcs_cm_control += src/PortMonitor.cc
libsrcs_cm_control += src/ChrDisplay.cc
#libsrcs_cm_control += src/CimBaseANTask.cc

libsrcs_cm_dtm += src/DtmInit.cc
libsrcs_cm_dtm += src/DtmShell.cc
libsrcs_cm_dtm += src/DtmConsoleTask.cc
libsrcs_cm_dtm += src/DtmMonitorTask.cc
libsrcs_cm_dtm += src/DtmConfigTask.cc

ifeq ($(have_rtems),$(true))
DEFINES += -D__INSIDE_RTEMS_BSD_TCPIP_STACK__
endif

#tgtsrcs_fm :=  src/fm.cc
#tgtlibs_fm :=  service/logger \
#               tool/exception \
#               tool/concurrency \
#               tool/time \
#               tool/map \
#               tool/mem \
#               tool/debug \
#               tool/string \
#               tool/io \
#               cm/cm_svc \
#               cm/cm_fm \
#               cm/cm_print \
#               cm/cm_net \
#               cm/cm_bpdu \
#               cm/cm_cntrl
#tgtslib_fm := rt

#tgtsrcs_fmd := src/fmd.cc
#tgtlibs_fmd := service/logger \
#               tool/exception \
#               tool/concurrency \
#               tool/time \
#               tool/map \
#               tool/mem \
#               tool/debug \
#               tool/string \
#               tool/io \
#               cm/cm_svc \
#               cm/cm_fm \
#               cm/cm_fmb \
#               cm/cm_net \
#               cm/cm_bpdu \
#               cm/cm_shell \
#               cm/cm_print \
#               cm/cm_cntrl              
#               cm/cm_syslog
#tgtslib_fmd := rt pthread

#tgtsrcs_fmbd := src/fmbdstat.cc
#tgtlibs_fmbd := service/logger \
#               tool/exception \
#               tool/concurrency \
#               tool/time \
#               tool/map \
#               tool/mem \
#               tool/debug \
#               tool/string \
#               tool/io \
#               cm/cm_svc \
#               cm/cm_fm \
#               cm/cm_fmb \
#               cm/cm_net \
#               cm/cm_bpdu \
#               cm/cm_shell \
#               cm/cm_print \
#               cm/cm_cntrl              
#               cm/cm_syslog
#tgtslib_fmbd := rt pthread

tgtsrcs_fmd := src/cmd.cc src/TahoeShell.cc src/BaliShell.cc
tgtlibs_fmd := service/logger \
               tool/exception \
               tool/concurrency \
               tool/time \
               tool/map \
               tool/mem \
               tool/debug \
               tool/string \
               tool/io \
               cm/cm_svc \
               cm/cm_fm \
               cm/cm_fmb \
               cm/cm_net \
               cm/cm_bpdu \
               cm/cm_shell \
               cm/cm_print \
               cm/cm_cntrl              
#               cm/cm_syslog
tgtslib_fmd := rt pthread

tgtsrcs_bsi_dump := src/bsi_dump.cc
tgtlibs_bsi_dump := tool/map tool/mem tool/debug cm/cm_svc cm/cm_print
tgtslib_bsi_dump := rt

#tgtsrcs_dtm := src/DtmConsoleTask.cc src/DtmMonitorTask.cc src/dtm.cc
tgtsrcs_dtm := src/dtm.cc
tgtlibs_dtm := oldPpi/bsdNet \
               oldPpi/net \
               cm_svc/cmb \
               cm_svc/fci \
               cm_svc/dynalink \
               cm_svc/shell \
               cm_svc/console \
               cm_svc/debug \
               cm_svc/logger \
               tool/concurrency \
               tool/exception \
               tool/io \
               tool/string \
               tool/time \
               cm/cm_svc \
               cm/cm_ffs \
               cm/cm_fm \
               cm/cm_bpdu \
               cm/cm_net \
               cm/cm_ipmi \
               cm/cm_xcf \
               cm/cm_smbus \
               cm/cm_phy \
               cm/cm_init \
               cm/cm_shell \
               cm/cm_log \
               cm/cm_control \
               cm/cm_dtm

# Even though librtemscpu is always searched we have to mention it
# here so that at least one search is done at the right time
# w.r.t. searches of libnfs.
tgtslib_dtm := $(RTEMS_DIR)/nfs \
               $(RTEMS_DIR)/rtemscpu \
               $(RTEMS_DIR)/telnetd \
               $(RTEMS_DIR)/rtemsbsp
managrs_dtm := io event msg sem ext region

tgtsrcs_cim := src/cim.cc
tgtlibs_cim := oldPpi/bsdNet \
               oldPpi/net \
               cm_svc/cmb \
               cm_svc/fci \
               cm_svc/dynalink \
               cm_svc/shell \
               cm_svc/console \
               cm_svc/debug \
               cm_svc/logger \
               tool/concurrency \
               tool/exception \
               tool/io \
               tool/string \
               tool/time \
               cm/cm_svc \
               cm/cm_ffs \
               cm/cm_fm \
               cm/cm_bpdu \
               cm/cm_net \
               cm/cm_ipmi \
               cm/cm_xcf \
               cm/cm_smbus \
               cm/cm_phy \
               cm/cm_init \
               cm/cm_shell \
               cm/cm_log \
               cm/cm_control
tgtslib_cim := $(RTEMS_DIR)/nfs \
               $(RTEMS_DIR)/rtemscpu \
               $(RTEMS_DIR)/telnetd \
               $(RTEMS_DIR)/rtemsbsp
managrs_cim := io event msg sem ext region

tgtsrcs_brdinit := src/brdinit.cc
tgtlibs_brdinit := oldPpi/net \
	               cm_svc/cmb \
                   cm_svc/fci \
                   cm_svc/dynalink \
                   cm_svc/debug \
                   cm_svc/logger \
                   tool/concurrency \
                   tool/exception \
                   tool/io \
                   tool/string \
                   tool/time \
                   cm/cm_svc \
                   cm/cm_ffs \
                   cm/cm_init
tgtslib_brdinit := $(RTEMS_DIR)/nfs \
                   $(RTEMS_DIR)/telnetd
managrs_brdinit := io event sem ext region

tgtsrcs_iic := src/iic.cc
tgtlibs_iic := oldPpi/bsdNet \
               cm_svc/cmb \
               oldPpi/net \
               cm_svc/fci \
               cm_svc/dynalink \
               cm_svc/debug \
               cm_svc/logger \
               tool/concurrency \
               tool/exception \
               tool/io \
               tool/string \
               tool/time \
               cm/cm_fm \
               cm/cm_svc \
               cm/cm_ffs \
               cm/cm_init
tgtslib_iic := $(RTEMS_DIR)/nfs \
               $(RTEMS_DIR)/rtemscpu \
               $(RTEMS_DIR)/telnetd \
               $(RTEMS_DIR)/rtemsbsp
managrs_iic := io event sem ext region

endif

ifneq ($(findstring i386-linux,$(tgt_arch)),)

tgtnames    := dtm_shell
tgtsrcs_dtm_shell := dtm_shell.cc
tgtslib_dtm_shell := rt

endif
