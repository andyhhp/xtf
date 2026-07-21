ALL_CATEGORIES     := special functional xsa utility in-development nested-svm

ALL_ENVIRONMENTS   := pv64 pv32pae hvm64 hvm32pae hvm32pse hvm32

PV_ENVIRONMENTS    := $(filter pv%,$(ALL_ENVIRONMENTS))
HVM_ENVIRONMENTS   := $(filter hvm%,$(ALL_ENVIRONMENTS))
32BIT_ENVIRONMENTS := $(filter pv32% hvm32%,$(ALL_ENVIRONMENTS))
64BIT_ENVIRONMENTS := $(filter pv64% hvm64%,$(ALL_ENVIRONMENTS))
SVM_ENVIRONMENTS   := hvm64

# $(env)_guest => pv or hvm mapping
$(foreach env,$(PV_ENVIRONMENTS),$(eval $(env)_guest := pv))
$(foreach env,$(HVM_ENVIRONMENTS),$(eval $(env)_guest := hvm))

# $(env)_arch => x86_32/64 mapping
$(foreach env,$(32BIT_ENVIRONMENTS),$(eval $(env)_arch := x86_32))
$(foreach env,$(64BIT_ENVIRONMENTS),$(eval $(env)_arch := x86_64))
