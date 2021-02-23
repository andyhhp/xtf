# Common makefile for arm

# Compilation recipe
# arm needs linking normally, then converting to a binary format
define build-$(ARCH)
	$(LD) $$(LDFLAGS_$(ARCH)) $$(DEPS-$(ARCH)) -o $$@-syms
	$(OBJCOPY) $$@-syms -O binary $$@
endef
