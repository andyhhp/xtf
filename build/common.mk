ALL_CATEGORIES     := special functional xsa utility in-development

ALL_ENVIRONMENTS   := pv64 pv32pae hvm64 hvm32pae hvm32pse hvm32

PV_ENVIRONMENTS    := $(filter pv%,$(ALL_ENVIRONMENTS))
HVM_ENVIRONMENTS   := $(filter hvm%,$(ALL_ENVIRONMENTS))
32BIT_ENVIRONMENTS := $(filter pv32% hvm32%,$(ALL_ENVIRONMENTS))
64BIT_ENVIRONMENTS := $(filter pv64% hvm64%,$(ALL_ENVIRONMENTS))

# $(env)_guest => pv or hvm mapping
$(foreach env,$(PV_ENVIRONMENTS),$(eval $(env)_guest := pv))
$(foreach env,$(HVM_ENVIRONMENTS),$(eval $(env)_guest := hvm))

# $(env)_arch => x86_32/64 mapping
$(foreach env,$(32BIT_ENVIRONMENTS),$(eval $(env)_arch := x86_32))
$(foreach env,$(64BIT_ENVIRONMENTS),$(eval $(env)_arch := x86_64))

comma := ,

COMMON_FLAGS := -pipe -nostdinc -I$(ROOT)/include -I$(ROOT)/arch/x86/include -MMD -MP

cc-option = $(shell if [ -z "`echo 'int p=1;' | $(CC) $(1) -c -o /dev/null -x c - 2>&1`" ]; \
			then echo y; else echo n; fi)

ld-option = $(shell if $(LD) -v $(1) >/dev/null 2>&1; then echo y; else echo n; fi)

# Arrange for assembly files to have a proper .note.GNU-stack section added,
# to silence warnings otherwise issued by GNU ld 2.39 and newer.
COMMON_AFLAGS-$(call cc-option,-Wa$(comma)--noexecstack) += -Wa,--noexecstack

# Suppress warnings about LOAD segments with RWX permissions, as what we build
# aren't normal user-mode executables.
LDFLAGS-$(call ld-option,--warn-rwx-segments) += --no-warn-rwx-segments

COMMON_AFLAGS := $(COMMON_FLAGS) $(COMMON_AFLAGS-y) -D__ASSEMBLY__
COMMON_CFLAGS := $(COMMON_FLAGS) $(COMMON_CFLAGS-y)
COMMON_CFLAGS += -Wall -Wextra -Werror -std=gnu99 -Wstrict-prototypes -O3 -g
COMMON_CFLAGS += -fno-common -fno-asynchronous-unwind-tables -fno-strict-aliasing
COMMON_CFLAGS += -fno-stack-protector -fno-pic -ffreestanding
COMMON_CFLAGS += -mno-red-zone -mno-sse
COMMON_CFLAGS += -Wno-unused-parameter -Winline

COMMON_AFLAGS-x86_32 := -m32
COMMON_AFLAGS-x86_64 := -m64

COMMON_CFLAGS-x86_32 := -m32
COMMON_CFLAGS-x86_64 := -m64

defcfg-pv    := $(ROOT)/config/default-pv.cfg.in
defcfg-hvm   := $(ROOT)/config/default-hvm.cfg.in

obj-perbits :=
obj-perenv  :=
include $(ROOT)/build/files.mk

# Run once per environment to set up some common bits & pieces
define PERENV_setup

AFLAGS_$($(1)_arch) := $$(COMMON_AFLAGS) $$(COMMON_AFLAGS-$($(1)_arch))
CFLAGS_$($(1)_arch) := $$(COMMON_CFLAGS) $$(COMMON_CFLAGS-$($(1)_arch))

AFLAGS_$(1) := $$(AFLAGS_$($(1)_arch)) $$(COMMON_AFLAGS-$(1)) -DCONFIG_ENV_$(1) -include arch/config.h
CFLAGS_$(1) := $$(CFLAGS_$($(1)_arch)) $$(COMMON_CFLAGS-$(1)) -DCONFIG_ENV_$(1) -include arch/config.h

link-$(1) := $(ROOT)/arch/x86/link-$(1).lds

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

$(foreach env,$(ALL_ENVIRONMENTS),$(eval $(call PERENV_setup,$(env))))

define move-if-changed
	if ! cmp -s $(1) $(2); then mv -f $(1) $(2); else rm -f $(1); fi
endef
