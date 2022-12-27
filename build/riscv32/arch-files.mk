# Architecture specific files compiled and linked for riscv32

# Include riscv common files
include $(ROOT)/build/riscv-common/arch-files.mk

# Specific files for riscv32
obj-perenv += $(ROOT)/arch/riscv/head.o
