MAKEFLAGS += -r
ROOT := $(abspath $(CURDIR)/../..)

include $(ROOT)/build/common.mk

NAME      := xsa-168
CATEGORY  := xsa
TEST-ENVS := hvm64

TEST-EXTRA-CFG := extra.cfg.in

obj-perenv += main.o

include $(ROOT)/build/gen.mk
