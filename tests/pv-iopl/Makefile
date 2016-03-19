MAKEFLAGS += -r
ROOT := $(abspath $(CURDIR)/../..)

include $(ROOT)/build/common.mk

NAME      := pv-iopl
CATEGORY  := functional
TEST-ENVS := $(PV_ENVIRONMENTS)

obj-perenv += main.o asm.o

include $(ROOT)/build/gen.mk
