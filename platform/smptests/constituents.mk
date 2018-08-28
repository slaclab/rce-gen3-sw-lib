no_dummy  = true
NOBLDINFO = true

ifeq ($(have_rtems),$(true))
tgtnames :=
tgtnames += smp01
tgtnames += smp02
tgtnames += smp03
tgtnames += smp04
tgtnames += smp05
tgtnames += smp06
tgtnames += smp07
tgtnames += smp08
tgtnames += smp09
#tgtnames += smpatomic01
#tgtnames += smpatomic02
#tgtnames += smpatomic03
#tgtnames += smpatomic04
#tgtnames += smpatomic05
#tgtnames += smpatomic06
#tgtnames += smpatomic07
tgtnames += smplock01
#tgtnames += smpschedule01

srcdir   := $(RTEMS_ROOT)/src/testsuites/smptests/

CPPFLAGS += -I$(RTEMS_ROOT)/src/testsuites/support/include
CPPFLAGS += -DSMPTEST

libs     := $(RTEMS_DIR)/rtemscpu
libs     += $(RTEMS_DIR)/rtemsbsp

tgtsrcs_smp01  	     := $(addprefix $(srcdir), smp01/init.c)
tgtsrcs_smp01  	     += $(addprefix $(srcdir), smp01/tasks.c)
tgtsrcs_smp01  	     += $(addprefix $(srcdir), ../support/src/locked_print.c)
tgtslibs_smp01 	     := $(libs)

tgtsrcs_smp02  	     := $(addprefix $(srcdir), smp02/init.c)
tgtsrcs_smp02  	     += $(addprefix $(srcdir), smp02/tasks.c)
tgtsrcs_smp02  	     += $(addprefix $(srcdir), ../support/src/locked_print.c)
tgtslibs_smp02 	     := $(libs)

tgtsrcs_smp03  	     := $(addprefix $(srcdir), smp03/init.c)
tgtsrcs_smp03  	     += $(addprefix $(srcdir), smp03/tasks.c)
tgtsrcs_smp03  	     += $(addprefix $(srcdir), ../support/src/locked_print.c)
tgtslibs_smp03 	     := $(libs)

tgtsrcs_smp04  	     := $(addprefix $(srcdir), smp04/init.c)
tgtsrcs_smp04  	     += $(addprefix $(srcdir), ../support/src/locked_print.c)
tgtslibs_smp04 	     := $(libs)

tgtsrcs_smp05  	     := $(addprefix $(srcdir), smp05/init.c)
tgtsrcs_smp05  	     += $(addprefix $(srcdir), ../support/src/locked_print.c)
tgtslibs_smp05 	     := $(libs)

tgtsrcs_smp06  	     := $(addprefix $(srcdir), smp06/init.c)
tgtsrcs_smp06  	     += $(addprefix $(srcdir), ../support/src/locked_print.c)
tgtslibs_smp06 	     := $(libs)

tgtsrcs_smp07  	     := $(addprefix $(srcdir), smp07/init.c)
tgtsrcs_smp07  	     += $(addprefix $(srcdir), ../support/src/locked_print.c)
tgtslibs_smp07 	     := $(libs)

tgtsrcs_smp08  	     := $(addprefix $(srcdir), smp08/init.c)
tgtsrcs_smp08  	     += $(addprefix $(srcdir), smp08/tasks.c)
tgtsrcs_smp08  	     += $(addprefix $(srcdir), ../support/src/locked_print.c)
tgtslibs_smp08 	     := $(libs)

tgtsrcs_smp09  	     := $(addprefix $(srcdir), smp09/init.c)
tgtsrcs_smp09  	     += $(addprefix $(srcdir), ../support/src/locked_print.c)
tgtslibs_smp09 	     := $(libs)

tgtsrcs_smpatomic01  := $(addprefix $(srcdir), smpatomic01/init.c)
tgtsrcs_smpatomic01  += $(addprefix $(srcdir), smpatomic01/tasks.c)
tgtsrcs_smpatomic01  += $(addprefix $(srcdir), ../support/src/locked_print.c)
tgtslibs_smpatomic01 := $(libs)

tgtsrcs_smpatomic02  := $(addprefix $(srcdir), smpatomic02/init.c)
tgtsrcs_smpatomic02  += $(addprefix $(srcdir), smpatomic02/tasks.c)
tgtsrcs_smpatomic02  += $(addprefix $(srcdir), ../support/src/locked_print.c)
tgtslibs_smpatomic02 := $(libs)

tgtsrcs_smpatomic03  := $(addprefix $(srcdir), smpatomic03/init.c)
tgtsrcs_smpatomic03  += $(addprefix $(srcdir), smpatomic03/tasks.c)
tgtsrcs_smpatomic03  += $(addprefix $(srcdir), ../support/src/locked_print.c)
tgtslibs_smpatomic03 := $(libs)

tgtsrcs_smpatomic04  := $(addprefix $(srcdir), smpatomic04/init.c)
tgtsrcs_smpatomic04  += $(addprefix $(srcdir), smpatomic04/tasks.c)
tgtsrcs_smpatomic04  += $(addprefix $(srcdir), ../support/src/locked_print.c)
tgtslibs_smpatomic04 := $(libs)

tgtsrcs_smpatomic05  := $(addprefix $(srcdir), smpatomic05/init.c)
tgtsrcs_smpatomic05  += $(addprefix $(srcdir), smpatomic05/tasks.c)
tgtsrcs_smpatomic05  += $(addprefix $(srcdir), ../support/src/locked_print.c)
tgtslibs_smpatomic05 := $(libs)

tgtsrcs_smpatomic06  := $(addprefix $(srcdir), smpatomic06/init.c)
tgtsrcs_smpatomic06  += $(addprefix $(srcdir), smpatomic06/tasks.c)
tgtsrcs_smpatomic06  += $(addprefix $(srcdir), ../support/src/locked_print.c)
tgtslibs_smpatomic06 := $(libs)

tgtsrcs_smpatomic07  := $(addprefix $(srcdir), smpatomic07/init.c)
tgtsrcs_smpatomic07  += $(addprefix $(srcdir), smpatomic07/tasks.c)
tgtsrcs_smpatomic07  += $(addprefix $(srcdir), ../support/src/locked_print.c)
tgtslibs_smpatomic07 := $(libs)

tgtsrcs_smplock01    := $(addprefix $(srcdir), smplock01/init.c)
tgtsrcs_smplock01    += $(addprefix $(srcdir), ../support/src/locked_print.c)
tgtslibs_smplock01   := $(libs)

tgtsrcs_smpsched01   := $(addprefix $(srcdir), smpsched01/init.c)
tgtsrcs_smpsched01   += $(addprefix $(srcdir), ../support/src/locked_print.c)
tgtslibs_smpsched01  := $(libs)

endif
