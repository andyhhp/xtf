# Architecture specific configuration for arm32

SUBARCH := arm
ALL_ENVIRONMENTS := 32le

$(foreach env,$(ALL_ENVIRONMENTS),$(eval $(env)_guest := arm32))
$(foreach env,$(ALL_ENVIRONMENTS),$(eval $(env)_arch := arm32))

defcfg-arm32 := $(ROOT)/config/default-arm.cfg.in

COMMON_CFLAGS += -march=armv7-a

# Include arm common makefile
include $(ROOT)/build/arm-common/arch-common.mk
