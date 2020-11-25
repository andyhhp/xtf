# Architecture specific files compiled and linked for x86

# Per bitness
obj-perbits += $(ROOT)/common/console.o
obj-perbits += $(ROOT)/common/exlog.o
obj-perbits += $(ROOT)/common/extable.o
obj-perbits += $(ROOT)/common/grant_table.o
obj-perbits += $(ROOT)/common/heapsort.o
obj-perbits += $(ROOT)/common/lib.o
obj-perbits += $(ROOT)/common/libc/stdio.o
obj-perbits += $(ROOT)/common/libc/string.o
obj-perbits += $(ROOT)/common/libc/vsnprintf.o
obj-perbits += $(ROOT)/common/report.o
obj-perbits += $(ROOT)/common/setup.o
obj-perbits += $(ROOT)/common/xenbus.o
obj-perbits += $(ROOT)/common/weak-defaults.o

# Per environment
obj-perenv += $(ROOT)/arch/x86/decode.o
obj-perenv += $(ROOT)/arch/x86/desc.o
obj-perenv += $(ROOT)/arch/x86/extable.o
obj-perenv += $(ROOT)/arch/x86/grant_table.o
obj-perenv += $(ROOT)/arch/x86/hypercall_page.o
obj-perenv += $(ROOT)/arch/x86/msr.o
obj-perenv += $(ROOT)/arch/x86/setup.o
obj-perenv += $(ROOT)/arch/x86/traps.o


# HVM specific objects
obj-hvm += $(ROOT)/arch/x86/apic.o
obj-hvm += $(ROOT)/arch/x86/hpet.o
obj-hvm += $(ROOT)/arch/x86/hvm/head.o
obj-hvm += $(ROOT)/arch/x86/hvm/pagetables.o
obj-hvm += $(ROOT)/arch/x86/hvm/traps.o
obj-hvm += $(ROOT)/arch/x86/io-apic.o

# Arguably common objects, but PV guests will have no interest in them.
obj-hvm += $(ROOT)/arch/x86/vmx.o
obj-hvm += $(ROOT)/arch/x86/x86-tss.o

$(foreach env,$(HVM_ENVIRONMENTS),$(eval obj-$(env) += $(obj-hvm)))


# PV specific objects
obj-pv  += $(ROOT)/arch/x86/pv/head.o
obj-pv  += $(ROOT)/arch/x86/pv/traps.o
$(foreach env,$(PV_ENVIRONMENTS),$(eval obj-$(env) += $(obj-pv)))


# 32bit specific objects
obj-32  += $(ROOT)/arch/x86/entry_32.o
$(foreach env,$(32BIT_ENVIRONMENTS),$(eval obj-$(env) += $(obj-32)))


# 64bit specific objects
obj-64  += $(ROOT)/arch/x86/entry_64.o
$(foreach env,$(64BIT_ENVIRONMENTS),$(eval obj-$(env) += $(obj-64)))
