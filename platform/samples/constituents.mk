no_dummy  = true
NOBLDINFO = true

ifeq ($(have_rtems),$(true))
tgtnames :=
#tgtnames += base_mp-node1
#tgtnames += base_mp-node2
tgtnames += base_sp
tgtnames += capture
tgtnames += cdtest
tgtnames += fileio
tgtnames += hello
tgtnames += iostream
tgtnames += loopback
tgtnames += minimum
#tgtnames += nsecs
tgtnames += paranoia
#tgtnames += pppd
tgtnames += ticker
tgtnames += unlimited

srcdir   := $(RTEMS_ROOT)/src/testsuites/samples/

CPPFLAGS += -I$(RTEMS_ROOT)/src/testsuites/support/include
CFLAGS   += -DNOSIGNAL -DCYGNUS -DNOMAIN

libs     := $(RTEMS_DIR)/rtemscpu
libs     += $(RTEMS_DIR)/rtemsbsp

tgtsrcs_base_mp-node1  := $(addprefix $(srcdir), base_mp/init.c)
tgtsrcs_base_mp-node1  += $(addprefix $(srcdir), base_mp/apptask.c)
tgtslibs_base_mp-node1 := $(libs)

tgtsrcs_base_mp-node2  := $(addprefix $(srcdir), base_mp/init.c)
tgtsrcs_base_mp-node2  += $(addprefix $(srcdir), base_mp/apptask.c)
tgtslibs_base_mp-node2 := $(libs)

tgtsrcs_base_sp        := $(addprefix $(srcdir), base_sp/init.c)
tgtsrcs_base_sp        += $(addprefix $(srcdir), base_sp/apptask.c)
tgtslibs_base_sp       := $(libs)

tgtsrcs_capture        := $(addprefix $(srcdir), capture/init.c)
tgtsrcs_capture        += $(addprefix $(srcdir), capture/test1.c)
tgtslibs_capture       := $(libs)

tgtsrcs_cdtest         := $(addprefix $(srcdir), cdtest/init.c)
tgtsrcs_cdtest         += $(addprefix $(srcdir), cdtest/main.c)
tgtslibs_cdtest        := $(libs)

tgtsrcs_fileio         := $(addprefix $(srcdir), fileio/init.c)
tgtslibs_fileio        := $(libs)

tgtsrcs_hello          := $(addprefix $(srcdir), hello/init.c)
tgtslibs_hello         := $(libs)

tgtsrcs_iostream       := $(addprefix $(srcdir), iostream/init.cc)
tgtslibs_iostream      := $(libs)
tgtslibs_iostream      := $(RTEMS_DIR)/rtems++

tgtsrcs_loopback       := $(addprefix $(srcdir), loopback/init.c)
tgtslibs_loopback      := $(libs)

tgtsrcs_minimum        := $(addprefix $(srcdir), minimum/init.c)
tgtslibs_minimum       := $(libs)

tgtsrcs_nsecs          := $(addprefix $(srcdir), nsecs/init.c)
tgtsrcs_nsecs          += $(addprefix $(srcdir), nsecs/empty.c)
tgtslibs_nsecs         := $(libs)

tgtsrcs_paranoia       := $(addprefix $(srcdir), paranoia/init.c)
tgtsrcs_paranoia       += $(addprefix $(srcdir), paranoia/paranoia.c)
tgtslibs_paranoia      := $(libs)
tgtslibs_paranoia      += m

tgtsrcs_pppd           := $(addprefix $(srcdir), pppd/init.c)
tgtsrcs_pppd           += $(addprefix $(srcdir), pppd/pppdapp.c)
tgtslibs_pppd          := $(libs)
tgtslibs_pppd          += $(RTEMS_DIR)/pppd

tgtsrcs_ticker         := $(addprefix $(srcdir), ticker/init.c)
tgtsrcs_ticker         += $(addprefix $(srcdir), ticker/tasks.c)
tgtslibs_ticker        := $(libs)
tgtslibs_ticker        += $(RTEMS_DIR)/telnetd

tgtsrcs_unlimited      := $(addprefix $(srcdir), unlimited/init.c)
tgtsrcs_unlimited      += $(addprefix $(srcdir), unlimited/test1.c)
tgtsrcs_unlimited      += $(addprefix $(srcdir), unlimited/test2.c)
tgtsrcs_unlimited      += $(addprefix $(srcdir), unlimited/test3.c)
tgtslibs_unlimited     := $(libs)

endif
