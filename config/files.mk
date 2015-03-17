# Files compiled and linked for different architectures and environments
#
# obj-perarch  get compiled once per architecture
# obj-perenv   get get compiled once for each environment
# obj-$(env)   are objects unique to a specific environment

obj-perarch += $(ROOT)/common/setup.o

# Always link hypercall_page.S last as it is a page of data replaced by the hyperisor
obj-perenv += $(ROOT)/arch/x86/hypercall_page.o
