include $(ROOT)/build/common.mk

NAME      := nested-svm-loadsave
CATEGORY  := nested-svm
TEST-ENVS := $(SVM_ENVIRONMENTS)

TEST-EXTRA-CFG := extra.cfg.in

obj-perenv += main.o $(ROOT)/common/nested-svm/setup-l2.o

include $(ROOT)/build/gen.mk
