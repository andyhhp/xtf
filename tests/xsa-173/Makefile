MAKEFLAGS += -r
ROOT := $(abspath $(CURDIR)/../..)

include $(ROOT)/build/common.mk

NAME      := xsa-173
CATEGORY  := xsa
TEST-ENVS := hvm64

obj-perenv += main.o

include $(ROOT)/build/gen.mk
