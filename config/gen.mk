
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

.PHONY: build
build: $(foreach env,$(TEST-ENVS),test-$(env)-$(NAME))

.PHONY: install install-each-env
install: install-each-env

define PERENV_build

test-$(1)-$(NAME): $$(DEPS-$(1)) $$(link-$(1))
	$$(LD) $$(LDFLAGS_$(1)) $$(DEPS-$(1)) -o $$@

-include $$(link-$(1):%.lds=%.d)
-include $$(DEPS-$(1):%.o=%.d)

.PHONY: install-$(1)
install-$(1): test-$(1)-$(NAME)
	@mkdir -p $(DESTDIR)
	install -m775 -p $$< $(DESTDIR)
install-each-env: install-$(1)

endef
$(foreach env,$(TEST-ENVS),$(eval $(call PERENV_build,$(env))))

.PHONY: clean
clean:
	find $(ROOT) \( -name "*.o" -o -name "*.d" \) -delete
	rm -f $(foreach env,$(TEST-ENVS),test-$(env)-$(NAME))

.PHONY: %var
%var:
	@echo "$* = $($*)"
