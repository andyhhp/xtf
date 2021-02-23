# Architecture specific configuration for arm64

ARCH_PATH := $(ROOT)/arch/arm
ALL_ENVIRONMENTS := arm64

arm64_arch   := arm64
arm64_guest  := arm64
defcfg-arm64 := $(ROOT)/config/default-arm.cfg.in

COMMON_CFLAGS += -march=armv8-a

# Include arm common makefile
include $(ROOT)/build/arm-common/arch-common.mk
