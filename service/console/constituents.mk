
ifeq ($(and $(have_rtems),$(have_arm)),$(true))

tasknames := console
tasksrcs_console := src/console.c
basetgt_console  := platform/rtems
basemod_console  := service/shell

endif
