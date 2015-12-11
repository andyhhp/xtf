# Files compiled and linked for different architectures and environments
#
# obj-perarch  get compiled once per architecture
# obj-perenv   get get compiled once for each environment
# obj-$(env)   are objects unique to a specific environment

obj-perarch += $(ROOT)/common/console.o
obj-perarch += $(ROOT)/common/lib.o
obj-perarch += $(ROOT)/common/libc/string.o
obj-perarch += $(ROOT)/common/libc/vsnprintf.o
obj-perarch += $(ROOT)/common/report.o
obj-perarch += $(ROOT)/common/setup.o

obj-perenv += $(ROOT)/arch/x86/desc.o
obj-perenv += $(ROOT)/arch/x86/setup.o
obj-perenv += $(ROOT)/arch/x86/traps.o

# Always link hypercall_page.S last as it is a page of data replaced by the hyperisor
obj-perenv += $(ROOT)/arch/x86/hypercall_page.o

# HVM specific objects
obj-hvm += $(ROOT)/arch/x86/hvm_pagetables.o
obj-hvm += $(ROOT)/arch/x86/hvm/traps.o

obj-hvm32 += $(obj-hvm)
obj-hvm32 += $(ROOT)/arch/x86/entry_32.o

obj-hvm64 += $(obj-hvm)
obj-hvm64 += $(ROOT)/arch/x86/entry_64.o

# PV specific objects
obj-pv  += $(ROOT)/arch/x86/pv/traps.o

obj-pv32 += $(obj-pv)
obj-pv32 += $(ROOT)/arch/x86/entry_32.o

obj-pv64 += $(obj-pv)
obj-pv64 += $(ROOT)/arch/x86/entry_64.o
