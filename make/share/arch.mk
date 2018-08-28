# The list of build architectures is created from the $(platforms) and $(codetypes)
# variables in $(RELEASE_DIR)/compilation.mk.  For each valid platform, the 
# corresponding codetype suffixes are added to create architectures.  In
# the case of the 'plain' platform, the suffix is null.

define addArch
ifneq (plain,$(2))
architectures += $(1)-$(2)
else
architectures += $(1)
endif
endef

# The double define here is a make trick to make sure that $(architectures) is only filled once.
architectures :=
$(foreach plat,$(platforms),$(foreach ctype,$(codetypes),$(eval $(call addArch,$(plat),$(ctype)))))
architectures := $(sort $(architectures))

