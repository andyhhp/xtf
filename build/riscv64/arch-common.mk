# Architecture specific configuration for riscv64

ARCH_PATH := $(ROOT)/arch/riscv
ALL_ENVIRONMENTS := riscv64

riscv64_arch   := riscv64
riscv64_guest  := riscv64
defcfg-riscv64 := $(ROOT)/config/default-riscv.cfg.in

# Include riscv common makefile
include $(ROOT)/build/riscv-common/arch-common.mk
