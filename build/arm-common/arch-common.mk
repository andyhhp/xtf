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

# Specify whether to use PL011 UART:
# CONFIG_PL011_UART=<y/n>
ifeq ($(CONFIG_PL011_UART), y)
COMMON_AFLAGS += -DCONFIG_PL011_UART
COMMON_CFLAGS += -DCONFIG_PL011_UART

# Specify the PL011 UART base address on the command line using:
# CONFIG_PL011_ADDRESS=<address>
ifndef CONFIG_PL011_ADDRESS
$(error "You must specify CONFIG_PL011_ADDRESS.")
else
COMMON_AFLAGS += -DCONFIG_PL011_ADDRESS=$(CONFIG_PL011_ADDRESS)
COMMON_CFLAGS += -DCONFIG_PL011_ADDRESS=$(CONFIG_PL011_ADDRESS)
endif

# Specify whether to enable early printk using PL011 UART
# Otherwise Xen debug console will be used to print boot messages
# CONFIG_PL011_EARLY_PRINTK=<y/n>
ifeq ($(CONFIG_PL011_EARLY_PRINTK), y)
COMMON_AFLAGS += -DCONFIG_PL011_EARLY_PRINTK
COMMON_CFLAGS += -DCONFIG_PL011_EARLY_PRINTK
endif

endif # CONFIG_PL011_UART
