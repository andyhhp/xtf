# Common makefile for riscv

COMMON_AFLAGS += -mno-relax
COMMON_CFLAGS += -mno-relax

# Compilation recipe
# riscv needs linking normally, then converting to a binary format
define build-$(ARCH)
	$(LD) $$(LDFLAGS_$(ARCH)) $$(DEPS-$(ARCH)) -o $$@-syms
	$(OBJCOPY) $$@-syms -O binary $$@
endef
