MAKEFLAGS += -r
ROOT := $(abspath $(CURDIR)/../..)

include $(ROOT)/build/common.mk

NAME      := example
CATEGORY  := special
TEST-ENVS := $(ALL_ENVIRONMENTS)

obj-perenv += main.o

include $(ROOT)/build/gen.mk
