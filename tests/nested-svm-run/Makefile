include $(ROOT)/build/common.mk

NAME      := nested-svm-run
CATEGORY  := nested-svm
TEST-ENVS := $(SVM_ENVIRONMENTS)

TEST-EXTRA-CFG := extra.cfg.in

obj-perenv += main.o
obj-perenv += $(ROOT)/common/nested-svm/setup-l2.o

include $(ROOT)/build/gen.mk
