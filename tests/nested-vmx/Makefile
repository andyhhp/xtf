include $(ROOT)/build/common.mk

NAME      := nested-vmx
CATEGORY  := in-development
TEST-ENVS := $(HVM_ENVIRONMENTS)

TEST-EXTRA-CFG := extra.cfg.in

obj-perenv += main.o msr.o util.o vmxon.o

include $(ROOT)/build/gen.mk
