ROOT := $(abspath $(CURDIR)/../..)

include $(ROOT)/build/common.mk

NAME      := swint-emulation
TEST-ENVS := hvm64 hvm32

obj-perenv += main.o lowlevel.o

include $(ROOT)/build/gen.mk
