no_dummy  = true
NOBLDINFO = true

ifeq ($(have_rtems),$(true))
tgtnames :=
tgtnames += spfatal26

srcdir   := $(RTEMS_ROOT)/src/testsuites/sptests/

CPPFLAGS += -I$(RTEMS_ROOT)/src/testsuites/support/include
CFLAGS   += -DNOSIGNAL -DCYGNUS -DNOMAIN

libs     := $(RTEMS_DIR)/rtemscpu
libs     += $(RTEMS_DIR)/rtemsbsp

tgtsrcs_spfatal26  := $(addprefix $(srcdir), spfatal26/init.c)
tgtslibs_spfatal26 := $(libs)

endif
