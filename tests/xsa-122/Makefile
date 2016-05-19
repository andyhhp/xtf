MAKEFLAGS += -r
ROOT := $(abspath $(CURDIR)/../..)

include $(ROOT)/build/common.mk

NAME      := xsa-122
CATEGORY  := xsa
TEST-ENVS := $(ALL_ENVIRONMENTS)

obj-perenv += main.o

include $(ROOT)/build/gen.mk
