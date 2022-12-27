# Architecture specific configuration for riscv32

ARCH_PATH := $(ROOT)/arch/riscv
ALL_ENVIRONMENTS := riscv32

riscv32_arch   := riscv32
riscv32_guest  := riscv32
defcfg-riscv32 := $(ROOT)/config/default-riscv.cfg.in

# Include riscv common makefile
include $(ROOT)/build/riscv-common/arch-common.mk
