DESTDIR ?= $(ROOT)/dist
PREFIX ?= $(ROOT)
CC ?= gcc
OBJCOPY ?= objcopy

ALL_CATEGORIES     := special functional xsa utility

ALL_ENVIRONMENTS   := pv64 pv32pae hvm64 hvm32pae hvm32pse hvm32

PV_ENVIRONMENTS    := $(filter pv%,$(ALL_ENVIRONMENTS))
HVM_ENVIRONMENTS   := $(filter hvm%,$(ALL_ENVIRONMENTS))
32BIT_ENVIRONMENTS := $(filter pv32% hvm32%,$(ALL_ENVIRONMENTS))
64BIT_ENVIRONMENTS := $(filter pv64% hvm64%,$(ALL_ENVIRONMENTS))

pv64_arch     := x86_64
pv32pae_arch  := x86_32
hvm64_arch    := x86_64
hvm32pae_arch := x86_32
hvm32pse_arch := x86_32
hvm32_arch    := x86_32

COMMON_FLAGS := -pipe -I$(ROOT)/include -MMD -MP

COMMON_AFLAGS := $(COMMON_FLAGS) -D__ASSEMBLY__
COMMON_CFLAGS := $(COMMON_FLAGS) -Wall -Wextra -Werror -std=gnu99 -Wstrict-prototypes -O3 -g
COMMON_CFLAGS += -fno-common -fno-asynchronous-unwind-tables -fno-strict-aliasing
COMMON_CFLAGS += -fno-stack-protector -ffreestanding
COMMON_CFLAGS += -mno-red-zone -mno-sse
COMMON_CFLAGS += -Wno-unused-parameter

COMMON_AFLAGS-x86_32 := -m32
COMMON_AFLAGS-x86_64 := -m64

COMMON_CFLAGS-x86_32 := -m32
COMMON_CFLAGS-x86_64 := -m64

head-pv64     := $(ROOT)/arch/x86/boot/head_pv64.o
head-pv32pae  := $(ROOT)/arch/x86/boot/head_pv32pae.o
head-hvm64    := $(ROOT)/arch/x86/boot/head_hvm64.o
head-hvm32pae := $(ROOT)/arch/x86/boot/head_hvm32pae.o
head-hvm32pse := $(ROOT)/arch/x86/boot/head_hvm32pse.o
head-hvm32    := $(ROOT)/arch/x86/boot/head_hvm32.o

defcfg-pv    := $(ROOT)/config/default-pv.cfg.in
defcfg-hvm   := $(ROOT)/config/default-hvm.cfg.in

defcfg-pv64     := $(defcfg-pv)
defcfg-pv32pae  := $(defcfg-pv)
defcfg-hvm64    := $(defcfg-hvm)
defcfg-hvm32pae := $(defcfg-hvm)
defcfg-hvm32pse := $(defcfg-hvm)
defcfg-hvm32    := $(defcfg-hvm)

obj-perarch :=
obj-perenv  :=
include $(ROOT)/build/files.mk

# Run once per environment to set up some common bits & pieces
define PERENV_setup

AFLAGS_$($(1)_arch) := $$(COMMON_AFLAGS) $$(COMMON_AFLAGS-$($(1)_arch))
CFLAGS_$($(1)_arch) := $$(COMMON_CFLAGS) $$(COMMON_CFLAGS-$($(1)_arch))

AFLAGS_$(1) := $$(AFLAGS_$($(1)_arch)) $$(COMMON_AFLAGS-$(1)) -DCONFIG_ENV_$(1) -include arch/x86/config.h
CFLAGS_$(1) := $$(CFLAGS_$($(1)_arch)) $$(COMMON_CFLAGS-$(1)) -DCONFIG_ENV_$(1) -include arch/x86/config.h

link-$(1) := $(ROOT)/arch/x86/link-$(1).lds

LDFLAGS_$(1) := -T $$(link-$(1))

# Needs to pick up test-provided obj-perenv and obj-perarch
DEPS-$(1) = $(head-$(1)) \
	$$(obj-perarch:%.o=%-$($(1)_arch).o) \
	$$(obj-$(1):%.o=%-$(1).o) $$(obj-perenv:%.o=%-$(1).o)

# Generate head with approprate flags
ifneq ($(findstring $(1),$(PV_ENVIRONMENTS)),)
# PV guests generate head_$(env).o from head_pv.S
%/head_$(1).o: %/head_pv.S
	$$(CC) $$(AFLAGS_$(1)) -c $$< -o $$@
endif
ifneq ($(findstring $(1),$(HVM_ENVIRONMENTS)),)
# HVM guests generate head_$(env).o from head_hvm.S
%/head_$(1).o: %/head_hvm.S
	$$(CC) $$(AFLAGS_$(1)) -c $$< -o $$@
endif

# Generate .lds with approprate flags
%/link-$(1).lds: %/link.lds.S
	$$(CC) -E $$(AFLAGS_$(1)) -P -C $$< -o $$@

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

