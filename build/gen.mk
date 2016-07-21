
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

.PHONY: build
build: $(foreach env,$(TEST-ENVS),test-$(env)-$(NAME) test-$(env)-$(NAME).cfg)
build: test-info.json

test-info.json: $(ROOT)/build/mkinfo.py FORCE
	@$(PYTHON) $< $@.tmp "$(NAME)" "$(CATEGORY)" "$(TEST-ENVS)"
	@if ! cmp -s $@ $@.tmp; then mv -f $@.tmp $@; else rm -f $@.tmp; fi

.PHONY: install install-each-env
install: install-each-env test-info.json
	@$(INSTALL_DIR) $(DESTDIR)/tests/$(NAME)
	$(INSTALL_DATA) test-info.json $(DESTDIR)/tests/$(NAME)

define PERENV_build

ifneq ($(1),hvm64)
# Generic link line for most environments
test-$(1)-$(NAME): $$(DEPS-$(1)) $$(link-$(1))
	$$(LD) $$(LDFLAGS_$(1)) $$(DEPS-$(1)) -o $$@
else
# hvm64 needs linking normally, then converting to elf32-x86-64
test-$(1)-$(NAME): $$(DEPS-$(1)) $$(link-$(1))
	$$(LD) $$(LDFLAGS_$(1)) $$(DEPS-$(1)) -o $$@.tmp
	$$(OBJCOPY) $$@.tmp -O elf32-x86-64 $$@
	rm -f $$@.tmp
endif

cfg-$(1) ?= $(defcfg-$(1))

test-$(1)-$(NAME).cfg: $$(cfg-$(1)) FORCE
	@{ cat $$< $(TEST-EXTRA-CFG) ;} | \
	sed -e "s/@@NAME@@/$$(NAME)/g" \
		-e "s/@@ENV@@/$(1)/g" \
		-e "s!@@PREFIX@@!$$(PREFIX)!g" \
		> $$@.tmp
	@if ! cmp -s $$@ $$@.tmp; then mv -f $$@.tmp $$@; else rm -f $$@.tmp; fi

-include $$(link-$(1):%.lds=%.d)
-include $$(DEPS-$(1):%.o=%.d)

.PHONY: install-$(1) install-$(1).cfg
install-$(1): test-$(1)-$(NAME)
	@$(INSTALL_DIR) $(DESTDIR)/tests/$(NAME)
	$(INSTALL_PROGRAM) $$< $(DESTDIR)/tests/$(NAME)

install-$(1).cfg: test-$(1)-$(NAME).cfg
	@$(INSTALL_DIR) $(DESTDIR)/tests/$(NAME)
	$(INSTALL_DATA) $$< $(DESTDIR)/tests/$(NAME)

install-each-env: install-$(1) install-$(1).cfg

endef
$(foreach env,$(TEST-ENVS),$(eval $(call PERENV_build,$(env))))

.PHONY: clean
clean:
	find $(ROOT) \( -name "*.o" -o -name "*.d" \) -delete
	rm -f $(foreach env,$(TEST-ENVS),test-$(env)-$(NAME) test-$(env)-$(NAME).cfg)

.PHONY: %var
%var:
	@echo "$* = $($*)"

.PHONY: FORCE
FORCE:
