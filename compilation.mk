# Compilation.mk stub

# Platforms
platforms :=
#platforms += i86-linux-32
platforms += i86-linux-64

platforms += arm-eabi-rceCA9
platforms += arm-eabi-zedCA9
platforms += arm-eabi-zcCA9
platforms += arm-eabi-cmCA9
platforms += arm-rtems-rceCA9
platforms += arm-linux-rceCA9
platforms += arm-linux-cmCA9

# Compilation switches
codetypes :=        # clear the name, just in case
codetypes += opt    # turn on optimization compilation (opt only)
codetypes += dbg    # turn on debug compilation (debug only)
#codetypes += dbgopt # turn on both debug and optimization
#codetypes += plain  # just compile it

verbose  := false
