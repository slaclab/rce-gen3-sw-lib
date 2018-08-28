#! /bin/sh
#This script fixes up rtems libraries for use in a shared library.
#fixup_rtems.sh <path_to_rtems_bld_libs> <path_to_output_dir>
#fixup_rtems.sh $RTEMS_ROOT/bld/arm-rtems4.11/c/xilinx_zynq_ml1 fixedup_rtems_libs

echo "Copying required RTEMS libraries to $2..."
#these are the required rtems libraries
cd $1
LIBS="
lib.a \
libblock.a      \
libc.a          \
libcpuuse.a     \
libcsupport.a   \
libdefaultfs.a  \
libdevnull.a    \
libdosfs.a      \
libfsmount.a    \
libimfs.a       \
libmonitor.a    \
libnetworking.a \
libnfs.a        \
librfs.a       \
librpc.a       \
librtems.a     \
libsapi.a      \
libscore.a     \
libscorecpu.a  \
libserialio.a  \
libstackchk.a \
libstringto.a \
libsupport.a  \
libuntar.a \
libutf8proc.a \
libuuid.a \
libxdr.a \
libposix.a \
libbsp.a \
libshell.a \
start.o \
libtelnetd.a \
libnfs.a \
libmd.a \
libcapture.a \
libi2c.a \
libz.a
"
for i in $LIBS
do
  cp `find . -name $i` $2
done

#now fixup some libraries
echo "Fixing up RTEMS libraries..."
cd $2
#THESE ARE DUPLICATES THAT ARE OVERRIDDEN BY THE DAT CORE AND SHOULD NOT BE INCLUDED IN rtems.so
arm-rtems4.11-ar -v -d libbsp.a libbsp_a-bspgetworkarea.o libbsp_a-bspstarthooks.o libbsp_a-zynq-uart.o libbsp_a-bspclean.o libbsp_a-bspstartmmu.o libbsp_a-cadence-i2c.o

#rtems libposix.a doubly includes certain object files:
#libposix_a-pthreadatfork.o
#libposix_a-pthreadkill.o
#libposix_a-pthreadsigmask.o
#libposix_a-psxpriorityisvalid.o
arm-rtems4.11-ar -v -d libposix.a libposix_a-pthreadatfork.o libposix_a-pthreadkill.o libposix_a-pthreadsigmask.o libposix_a-psxpriorityisvalid.o

#extract network dependent shell commands to .o files - these will be used later in the network shell command .so
arm-rtems4.11-ar -v -x libshell.a libshell_a-main_netstats.o libshell_a-main_route.o libshell_a-main_ifconfig.o libshell_a-main_ping.o libshell_a-login_check.o

#remove duplicate default shell configuration in libshell_a-shellconfig.o, remove network dependent commands
arm-rtems4.11-ar -v -d libshell.a libshell_a-main_netstats.o libshell_a-main_route.o libshell_a-main_ifconfig.o libshell_a-main_ping.o libshell_a-login_check.o libshell_a-shellconfig.o

#remove network commands from monitor
arm-rtems4.11-ar -v -d libmonitor.a mon-network.o

#remove unused block devices - ramdisk, nvram, flash
arm-rtems4.11-ar -v -d libblock.a ramdisk-config.o ramdisk-driver.o ramdisk-init.o ramdisk-register.o flashdisk.o nvdisk-sram.o nvdisk.o

#remove ip_fw from libnetworking - externs undefined securelevel
arm-rtems4.11-ar -v -d libnetworking.a libnetworking_a-ip_fw.o

#remove rtime, dtablesize
arm-rtems4.11-ar -v -d librpc.a librpc_a-rpc_dtablesize.o librpc_a-rtime.o librpc_a-rpcdname.o librpc_a-netname.o librpc_a-netnamer.o

#remove unused CBS scheduler
arm-rtems4.11-ar -v -d  libscore.a \
libscore_a-schedulercbs.o \
libscore_a-schedulercbsattachthread.o \
libscore_a-schedulercbscleanup.o \
libscore_a-schedulercbscreateserver.o \
libscore_a-schedulercbsdestroyserver.o \
libscore_a-schedulercbsdetachthread.o \
libscore_a-schedulercbsgetapprovedbudget.o \
libscore_a-schedulercbsgetexecutiontime.o \
libscore_a-schedulercbsgetparameters.o \
libscore_a-schedulercbsgetremainingbudget.o \
libscore_a-schedulercbsgetserverid.o \
libscore_a-schedulercbsreleasejob.o \
libscore_a-schedulercbssetparameters.o \
libscore_a-schedulercbsunblock.o

#remove unused low and high exception handlers - contains undefined symbol SWI_Handler
arm-rtems4.11-ar -v -d  libscorecpu.a libscorecpu_a-arm_exc_handler_low.o libscorecpu_a-arm_exc_handler_high.o 

#disable nfs mbuf usage - remove xdr_mbuf.o sock_mbuf.o rpcio.o, a modified rpcio.c is now in the service/nfs package.
arm-rtems4.11-ar -v -d  libnfs.a libnfs_a-xdr_mbuf.o libnfs_a-sock_mbuf.o libnfs_a-rpcio.o

