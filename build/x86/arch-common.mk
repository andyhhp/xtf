# Architecture specific configuration for x86

# BASE_ARCH is the architecture name devoided of information about bitness.
# It may be that BASE_ARCH == ARCH for some architectures.
BASE_ARCH          := x86
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

COMMON_CFLAGS += -mno-red-zone -mno-sse

COMMON_AFLAGS-x86_32 := -m32
COMMON_AFLAGS-x86_64 := -m64
COMMON_CFLAGS-x86_32 := -m32
COMMON_CFLAGS-x86_64 := -m64

hvm64-format := $(firstword $(filter elf32-x86-64,$(shell $(OBJCOPY) --help)) elf32-i386)

# Compilation recipe for hvm64
# hvm64 needs linking normally, then converting to elf32-x86-64 or elf32-i386
define build-hvm64
	$(LD) $$(LDFLAGS_hvm64) $$(DEPS-hvm64) -o $$@.tmp
	$(OBJCOPY) $$@.tmp -O $(hvm64-format) $$@
	rm -f $$@.tmp
endef
