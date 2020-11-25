# Architecture independent/common configuration

ALL_CATEGORIES := special functional xsa utility in-development
COMMON_FLAGS := -pipe -I$(ROOT)/include -I$(ROOT)/arch/$(ARCH)/include -MMD -MP

cc-option = $(shell if [ -z "`echo 'int p=1;' | $(CC) $(1) -S -o /dev/null -x c - 2>&1`" ]; \
			then echo y; else echo n; fi)

# Disable PIE, but need to check if compiler supports it
COMMON_CFLAGS-$(call cc-option,-no-pie) += -no-pie

COMMON_AFLAGS := $(COMMON_FLAGS) -D__ASSEMBLY__
COMMON_CFLAGS := $(COMMON_FLAGS) $(COMMON_CFLAGS-y)

# Include architecture specific configuration
include $(ROOT)/build/$(ARCH)/arch-common.mk

COMMON_CFLAGS += -I$(ARCH_PATH)/include
COMMON_AFLAGS += -I$(ARCH_PATH)/include
COMMON_CFLAGS += -Wall -Wextra -Werror -std=gnu99 -Wstrict-prototypes -O3 -g
COMMON_CFLAGS += -fno-common -fno-asynchronous-unwind-tables -fno-strict-aliasing
COMMON_CFLAGS += -fno-stack-protector -fno-pic -ffreestanding -nostdinc
COMMON_CFLAGS += -Wno-unused-parameter -Winline

# Default guest configfiles
defcfg-pv    := $(ROOT)/config/default-pv.cfg.in
defcfg-hvm   := $(ROOT)/config/default-hvm.cfg.in

# Following variables needs to be set up in $(ROOT)/build/$(ARCH)/arch-files.mk
# obj-perbits  get compiled once per bitness
# obj-perenv   get compiled once for each environment
# obj-$(env)   are objects unique to a specific environment
obj-perbits :=
obj-perenv  :=

include $(ROOT)/build/$(ARCH)/arch-files.mk

# Set up some common bits and pieces for specified environment
define PERENV_setup

AFLAGS_$($(1)_arch) := $$(COMMON_AFLAGS) $$(COMMON_AFLAGS-$($(1)_arch))
CFLAGS_$($(1)_arch) := $$(COMMON_CFLAGS) $$(COMMON_CFLAGS-$($(1)_arch))

AFLAGS_$(1) := $$(AFLAGS_$($(1)_arch)) $$(COMMON_AFLAGS-$(1)) -DCONFIG_ENV_$(1) -include arch/config.h
CFLAGS_$(1) := $$(CFLAGS_$($(1)_arch)) $$(COMMON_CFLAGS-$(1)) -DCONFIG_ENV_$(1) -include arch/config.h

link-$(1) := $(ARCH_PATH)/link-$(1).lds

LDFLAGS_$(1) := -T $$(link-$(1)) -nostdlib $(LDFLAGS-y)

# Needs to pick up test-provided obj-perenv and obj-perbits
DEPS-$(1) = \
	$$(obj-perbits:%.o=%-$($(1)_arch).o) \
	$$(obj-$(1):%.o=%-$(1).o) $$(obj-perenv:%.o=%-$(1).o)

# Generate .lds with appropriate flags
%/link-$(1).lds: %/link.lds.S
	$$(CPP) $$(AFLAGS_$(1)) -P -C $$< -o $$@

# Generate a per-arch .o from .S
%-$($(1)_arch).o: %.S
	$$(CC) $$(AFLAGS_$($(1)_arch)) -c $$< -o $$@

# Generate a per-arch .o from .c
%-$($(1)_arch).o: %.c
	$$(CC) $$(CFLAGS_$($(1)_arch)) -c $$< -o $$@

# Generate a per-env .o from .S
%-$(1).o: %.S
	$$(CC) $$(AFLAGS_$(1)) -c $$< -o $$@

# Generate a per-env .o from .c
%-$(1).o: %.c
	$$(CC) $$(CFLAGS_$(1)) -c $$< -o $$@

endef

# Make a call to a function PERENV_setup once per each environment
$(foreach env,$(ALL_ENVIRONMENTS),$(eval $(call PERENV_setup,$(env))))

define move-if-changed
	if ! cmp -s $(1) $(2); then mv -f $(1) $(2); else rm -f $(1); fi
endef
