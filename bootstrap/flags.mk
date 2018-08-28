# We need to build this project for the special case of EABI boards as well
platforms += ppc-eabi-rce405
platforms += ppc-eabi-rce440
platforms += arm-eabi-cmCA9
#platforms += arm-eabi-rceCA9
platforms += arm-eabi-zcCA9
platforms += arm-eabi-zedCA9

# The build information should not be created for the bootstrap project since
# it doesn't presume a C++ or C runtime environment.
# Don't produce libdummy, either.
NOBLDINFO = true
no_dummy  = true

include $(RELEASE_DIR)/make/sw/flags.mk
