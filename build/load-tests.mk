include $(ROOT)/build/core.mk

TEST_MAKEFILES := $(wildcard $(TESTS:%=%/Makefile))
REGISTERED_TESTS :=

define xtf_metadata_key
$(subst -,_,$(subst /,_,$(1)))
endef

define xtf_reset_test_metadata
NAME :=
CATEGORY :=
TEST-ENVS :=
TEST-EXTRA-CFG :=
VARY-CFG :=
VCPUS :=
obj-perenv :=
endef

define xtf_canonicalise_test_obj
$(if $(filter $(ROOT)/% /%,$(1)),$(1),$(CURRENT_TEST_DIR)/$(1))
endef

define xtf_load_test
$$(eval $$(call xtf_reset_test_metadata))
CURRENT_TEST_DIR := $$(patsubst %/Makefile,%,$(1))
XTF_METADATA_ONLY := 1
include $(1)
TEST_KEY := $$(call xtf_metadata_key,$$(CURRENT_TEST_DIR))
REGISTERED_TESTS += $$(TEST_KEY)
TEST_DIR_$$(TEST_KEY) := $$(CURRENT_TEST_DIR)
TEST_NAME_$$(TEST_KEY) := $$(NAME)
TEST_CATEGORY_$$(TEST_KEY) := $$(CATEGORY)
TEST_ENVS_$$(TEST_KEY) := $$(TEST-ENVS)
TEST_EXTRA_CFG_$$(TEST_KEY) := $$(if $$(TEST-EXTRA-CFG), \
	$$(CURRENT_TEST_DIR)/$$(TEST-EXTRA-CFG))
TEST_VARY_CFG_$$(TEST_KEY) := $$(VARY-CFG)
TEST_VCPUS_$$(TEST_KEY) := $$(if $$(VCPUS),$$(VCPUS),1)
TEST_LOCAL_OBJ_PERENV_$$(TEST_KEY) := \
	$$(foreach obj,$$(obj-perenv), \
		$$(call xtf_canonicalise_test_obj,$$(obj)))
undefine TEST_KEY
undefine XTF_METADATA_ONLY
undefine CURRENT_TEST_DIR
endef

$(foreach test_makefile,$(TEST_MAKEFILES), \
	$(eval $(call xtf_load_test,$(test_makefile))))
