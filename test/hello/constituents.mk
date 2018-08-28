ifeq ($(have_rce),$(true))
tgtnames := hello

srcdir := src/

tgtsrcs_hello := $(addprefix $(srcdir), init.c)
tgtslibs_hello := \
    $(RTEMS_DIR)/nfs \
    $(RTEMS_DIR)/rtemscpu \
    $(RTEMS_DIR)/telnetd \
    $(RTEMS_DIR)/rtemsbsp
endif
