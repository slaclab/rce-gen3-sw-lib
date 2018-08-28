do_not_document = $(true)

# CM level images
tgtnames :=
libnames := cm_shell
ifeq ($(have_arm),$(true))
libsrcs_cm_shell += src/ShellCommon.cc
libsrcs_cm_shell += src/libtelnet.c
else
libsrcs_cm_shell += src/ShellCommands.cc
libsrcs_cm_shell += src/etherstat.cc
libsrcs_cm_shell += src/priority.cc
endif