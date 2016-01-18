ROOT := $(abspath $(CURDIR)/../..)

include $(ROOT)/build/common.mk

NAME      := swint-emulation
TEST-ENVS := $(HVM_ENVIRONMENTS)

obj-perenv += main.o lowlevel.o

include $(ROOT)/build/gen.mk
