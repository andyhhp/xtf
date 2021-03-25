# Architecture specific files compiled and linked for arm32

# Include arm common files
include $(ROOT)/build/arm-common/arch-files.mk

# Specific files for arm32
obj-perenv += $(ROOT)/arch/arm/arm32/head.o
obj-perenv += $(ROOT)/arch/arm/arm32/hypercall.o
