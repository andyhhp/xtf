# Architecture independent makefile for compiling tests

# Sanity checking of expected parameters
ifeq ($(NAME),)
$(error NAME should be specified)
endif

ifeq ($(TEST-ENVS),)
$(error TEST-ENVS should not be empty)
endif

ifneq ($(filter-out $(ALL_ENVIRONMENTS),$(TEST-ENVS)),)
$(error Unrecognised environments '$(filter-out $(ALL_ENVIRONMENTS),$(TEST-ENVS))')
endif

ifeq ($(CATEGORY),)
$(error CATEGORY should not be empty)
endif

ifneq ($(filter-out $(ALL_CATEGORIES),$(CATEGORY)),)
$(error Unrecognised category '$(filter-out $(ALL_CATEGORIES),$(CATEGORY))')
endif

ifeq ($(VCPUS),)
VCPUS := 1 # Default to 1 vcpu if not provided
endif

ifneq ($(VARY-CFG),)
TEST-CFGS := $(foreach env,$(TEST-ENVS),$(foreach vary,$(VARY-CFG),test-$(env)-$(NAME)~$(vary).cfg))
else
TEST-CFGS := $(foreach env,$(TEST-ENVS),test-$(env)-$(NAME).cfg)
endif

.PHONY: build
build: $(foreach env,$(TEST-ENVS),test-$(env)-$(NAME)) $(TEST-CFGS)
build: info.json

info.json: $(ROOT)/build/mkinfo.py FORCE
	@$(PYTHON) $< $@.tmp "$(NAME)" "$(CATEGORY)" "$(TEST-ENVS)" "$(VARY-CFG)"
	@$(call move-if-changed,$@.tmp,$@)

.PHONY: install install-each-env
install: install-each-env info.json
	@$(INSTALL_DIR) $(DESTDIR)$(xtftestdir)/$(NAME)
	$(INSTALL_DATA) info.json $(DESTDIR)$(xtftestdir)/$(NAME)

# Build a test for specified environment
define PERENV_build

# If any environment needs a special compilation/linking recipe instead of
# the default one, a custom recipe called build-$(env) e.g. build-hvm64
# should be created in $(ROOT)/build/$(ARCH)/arch-common.mk

test-$(1)-$(NAME): $$(DEPS-$(1)) $$(link-$(1))
ifndef build-$(1)
	@# Generic link line for most environments
	$(LD) $$(LDFLAGS_$(1)) $$(DEPS-$(1)) -o $$@
else
	@# Environment specific compilation recipe
	$(call build-$(1))
endif

cfg-$(1) ?= $(defcfg-$($(1)_guest))

cfg-default-deps := $(ROOT)/build/mkcfg.py $$(cfg-$(1)) $(TEST-EXTRA-CFG) FORCE

test-$(1)-$(NAME).cfg: $$(cfg-default-deps)
	$(PYTHON) $$< $$@.tmp "$$(cfg-$(1))" "$(VCPUS)" "$(TEST-EXTRA-CFG)" ""
	@$(call move-if-changed,$$@.tmp,$$@)

test-$(1)-$(NAME)~%.cfg: $$(cfg-default-deps) %.cfg.in
	$(PYTHON) $$< $$@.tmp "$$(cfg-$(1))" "$(VCPUS)" "$(TEST-EXTRA-CFG)" "$$*.cfg.in"
	@$(call move-if-changed,$$@.tmp,$$@)

test-$(1)-$(NAME)~%.cfg: $$(cfg-default-deps) $(ROOT)/config/%.cfg.in
	$(PYTHON) $$< $$@.tmp "$$(cfg-$(1))" "$(VCPUS)" "$(TEST-EXTRA-CFG)" "$(ROOT)/config/$$*.cfg.in"
	@$(call move-if-changed,$$@.tmp,$$@)

-include $$(link-$(1):%.lds=%.d)
-include $$(DEPS-$(1):%.o=%.d)

.PHONY: install-$(1) install-$(1).cfg
install-$(1): test-$(1)-$(NAME)
	@$(INSTALL_DIR) $(DESTDIR)$(xtftestdir)/$(NAME)
	$(INSTALL_PROGRAM) $$< $(DESTDIR)$(xtftestdir)/$(NAME)

install-$(1).cfg: $(filter test-$(1)-%,$(TEST-CFGS))
	@$(INSTALL_DIR) $(DESTDIR)$(xtftestdir)/$(NAME)
	$(INSTALL_DATA) $$^ $(DESTDIR)$(xtftestdir)/$(NAME)

install-each-env: install-$(1) install-$(1).cfg

endef

# Make a call to a function PERENV_build once per each test's environment
$(foreach env,$(TEST-ENVS),$(eval $(call PERENV_build,$(env))))

.PHONY: clean
clean:
	find $(ROOT) \( -name "*.o" -o -name "*.d" \) -delete
	rm -f $(foreach env,$(TEST-ENVS),test-$(env)-$(NAME) test-$(env)-$(NAME)*.cfg)

.PHONY: %var
%var:
	@echo "$* = $($*)"

.PHONY: FORCE
FORCE:
