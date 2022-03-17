# Common makefile for arm

# Compilation recipe
# arm needs linking normally, then converting to a binary format
define build-arm
	$(LD) $$(LDFLAGS_$(1)) $$(DEPS-$(1)) -o $$@-syms
	$(OBJCOPY) $$@-syms -O binary $$@
endef
