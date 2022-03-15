# Architecture specific files compiled and linked for arm64

# Include arm common files
include $(ROOT)/build/arm-common/arch-files.mk

# Specific files for arm64
obj-perenv += $(ROOT)/arch/arm/arm64/head.o
obj-perenv += $(ROOT)/arch/arm/arm64/cache.o
