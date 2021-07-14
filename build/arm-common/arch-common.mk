# Common makefile for arm

# Compilation recipe
# arm needs linking normally, then converting to a binary format
define build-arm
	$(LD) $$(LDFLAGS_$(1)) $$(DEPS-$(1)) -o $$@-syms
	$(OBJCOPY) $$@-syms -O binary $$@
endef

# Specify the load address on the command line using:
# CONFIG_LOAD_ADDRESS=<address>
ifdef CONFIG_LOAD_ADDRESS
COMMON_AFLAGS += -DCONFIG_LOAD_ADDRESS=$(CONFIG_LOAD_ADDRESS)
endif
