# Architecture specific configuration for arm32

ARCH_PATH := $(ROOT)/arch/arm
ALL_ENVIRONMENTS := arm32

arm32_arch   := arm32
arm32_guest  := arm32
defcfg-arm32 := $(ROOT)/config/default-arm.cfg.in

COMMON_CFLAGS += -march=armv7-a

# Include arm common makefile
include $(ROOT)/build/arm-common/arch-common.mk
