# List of packages (low level first)
ifeq ($(have_cm),$(true))
ifeq ($(have_linux),$(true))
packages := cm_svc
packages += fm
packages += fmb
packages += bpdu
packages += cm_net
packages += cm_shell
packages += cm_control
else
ifeq ($(have_rtems),$(true))
packages := cm_svc
packages += fm
packages += bpdu
packages += cm_net
packages += cm_shell
packages += cm_control
packages += ffs
packages += phy
packages += xcf
packages += smbus
packages += ipmi
#packages += mon
packages += init
packages += log
endif
endif
endif

# Add for all combinations.  However, the boot constituents file 
# only builds for arm-eabi.  --JHP 20140410
packages += boot


#ifneq ($(findstring i386-linux,$(tgt_arch)),)
#packages := fm
##packages += mon
#packages += log
#endif
