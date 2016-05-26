include $(ROOT)/build/common.mk

NAME      := pagetable-emulation
CATEGORY  := functional
TEST-ENVS := $(HVM_ENVIRONMENTS)

VARY-CFG  := hap shadow

obj-perenv += stubs.o main.o

include $(ROOT)/build/gen.mk
