# Common files compiled and linked for arm

obj-perenv += $(ROOT)/common/console.o
obj-perenv += $(ROOT)/common/lib.o
obj-perenv += $(ROOT)/common/libc/stdio.o
obj-perenv += $(ROOT)/common/libc/string.o
obj-perenv += $(ROOT)/common/libc/vsnprintf.o
obj-perenv += $(ROOT)/common/report.o
obj-perenv += $(ROOT)/common/setup.o
obj-perenv += $(ROOT)/common/xenbus.o

obj-perenv += $(ROOT)/arch/arm/decode.o
obj-perenv += $(ROOT)/arch/arm/setup.o
obj-perenv += $(ROOT)/arch/arm/traps.o
