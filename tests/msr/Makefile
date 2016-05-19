MAKEFLAGS += -r
ROOT := $(abspath $(CURDIR)/../..)

include $(ROOT)/build/common.mk

NAME      := msr
CATEGORY  := utility
TEST-ENVS := $(ALL_ENVIRONMENTS)

obj-perenv += main.o

include $(ROOT)/build/gen.mk
