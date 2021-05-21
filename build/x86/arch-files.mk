# Architecture specific files compiled and linked for x86

# Per architecture
obj-perarch += $(ROOT)/common/console.o
obj-perarch += $(ROOT)/common/exlog.o
obj-perarch += $(ROOT)/common/extable.o
obj-perarch += $(ROOT)/common/grant_table.o
obj-perarch += $(ROOT)/common/heapsort.o
obj-perarch += $(ROOT)/common/lib.o
obj-perarch += $(ROOT)/common/libc/stdio.o
obj-perarch += $(ROOT)/common/libc/string.o
obj-perarch += $(ROOT)/common/libc/vsnprintf.o
obj-perarch += $(ROOT)/common/report.o
obj-perarch += $(ROOT)/common/setup.o
obj-perarch += $(ROOT)/common/xenbus.o

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
obj-hvm += $(ROOT)/arch/x86/hvm/pagetables.o
obj-hvm += $(ROOT)/arch/x86/hvm/traps.o
obj-hvm += $(ROOT)/arch/x86/io-apic.o

# Arguably common objects, but PV guests will have no interest in them.
obj-hvm += $(ROOT)/arch/x86/vmx.o
obj-hvm += $(ROOT)/arch/x86/x86-tss.o

$(foreach env,$(HVM_ENVIRONMENTS),$(eval obj-$(env) += $(obj-hvm)))

# PV specific objects
obj-pv  += $(ROOT)/arch/x86/pv/traps.o
$(foreach env,$(PV_ENVIRONMENTS),$(eval obj-$(env) += $(obj-pv)))

# 32bit specific objects
obj-32  += $(ROOT)/arch/x86/entry_32.o
$(foreach env,$(32BIT_ENVIRONMENTS),$(eval obj-$(env) += $(obj-32)))

# 64bit specific objects
obj-64  += $(ROOT)/arch/x86/entry_64.o
$(foreach env,$(64BIT_ENVIRONMENTS),$(eval obj-$(env) += $(obj-64)))
