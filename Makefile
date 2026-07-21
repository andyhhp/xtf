MAKEFLAGS += -rR
ROOT := $(abspath $(CURDIR))
export ROOT

# Default to the all rule
all:

# Local settings and rules
-include Makefile.local

# $(xtfdir) defaults to $(ROOT) so development and testing can be done
# straight out of the working tree.
xtfdir  ?= $(ROOT)
DESTDIR ?= $(ROOT)/dist

ifeq ($(filter /%,$(xtfdir)),)
$(error $$(xtfdir) must be absolute, not '$(xtfdir)')
endif

ifneq ($(DESTDIR),)
ifeq ($(filter /%,$(DESTDIR)),)
$(error $$(DESTDIR) must be absolute, not '$(DESTDIR)')
endif
endif

xtftestdir := $(xtfdir)/tests

export DESTDIR xtfdir xtftestdir

ifeq ($(LLVM),) # GCC toolchain
CC              := $(CROSS_COMPILE)gcc
LD              := $(CROSS_COMPILE)ld
OBJCOPY         := $(CROSS_COMPILE)objcopy

else # LLVM toolchain

# Optional -$NUM version when multiple toolchains are installed
ver := $(filter -%,$(LLVM))
CC              := clang$(ver) $(if $(CROSS_COMPILE),--target=$(notdir $(CROSS_COMPILE:%-=%)))
LD              := ld.lld$(ver)
OBJCOPY         := llvm-objcopy$(ver)
undefine ver

endif

CPP             := $(CC) -E
INSTALL         := install
INSTALL_DATA    := $(INSTALL) -m 644 -p
INSTALL_DIR     := $(INSTALL) -d -p
INSTALL_PROGRAM := $(INSTALL) -p

# Best effort attempt to find a python interpreter, defaulting to Python 3 if
# available.  Fall back to just `python`.
PYTHON_INTERPRETER := $(word 1,$(shell command -v python3 || command -v python || command -v python2) python)
PYTHON             ?= $(PYTHON_INTERPRETER)

export CC LD CPP INSTALL INSTALL_DATA INSTALL_DIR INSTALL_PROGRAM OBJCOPY PYTHON

# By default enable all the tests
TESTS ?= $(wildcard $(ROOT)/tests/*)

# Prefer Ninja when it is available, but keep the recursive make path as the
# fallback and as an explicit override via USE_MAKE=1.
NINJA_AVAILABLE := $(if $(shell command -v ninja 2>/dev/null),1,0)
USE_MAKE ?= $(if $(NINJA_AVAILABLE),0,1)

ACTIVE_GOALS := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),all)
NINJA_GOALS := ninja-vars ninja-file ninja-build ninja-install
METADATA_GOALS := $(NINJA_GOALS)
COMMON_GOALS := $(NINJA_GOALS)

ifeq ($(USE_MAKE),0)
METADATA_GOALS += all install
COMMON_GOALS += all install
endif

ifneq ($(filter $(METADATA_GOALS),$(ACTIVE_GOALS)),)
include $(ROOT)/build/load-tests.mk
endif

ifneq ($(filter $(COMMON_GOALS),$(ACTIVE_GOALS)),)
include $(ROOT)/build/common.mk
endif

# Convert the selected test directories into explicit top-level targets so GNU
# make can schedule independent tests in parallel, rather than hiding the work
# behind one shell loop.

TEST_MAKEFILES := $(wildcard $(TESTS:%=%/Makefile))
BUILD_TARGETS := $(patsubst %/Makefile,%/.build,$(TEST_MAKEFILES))
INSTALL_TARGETS := $(patsubst %/Makefile,%/.install,$(TEST_MAKEFILES))

# Multiple test sub-makes rebuild the same objects under common/ and arch/.
# Seed those shared artefacts once before the parallel fan-out, but skip the
# bootstrap entirely when only one test was selected so TESTS filtering keeps
# its expected no-op behaviour.

ifneq ($(word 2,$(BUILD_TARGETS)),)
SHARED_BOOTSTRAP_TARGET := $(firstword $(BUILD_TARGETS:.build=.shared-ready))
endif

ifeq ($(USE_MAKE),0)

.PHONY: all install
all: ninja-build

install: ninja-install

else

.PHONY: all $(BUILD_TARGETS) $(INSTALL_TARGETS)
all: $(SHARED_BOOTSTRAP_TARGET) $(BUILD_TARGETS)

# Leading '+' preserves jobserver recursion when the parent was run with -j.
$(SHARED_BOOTSTRAP_TARGET):
	+$(MAKE) -C $(@D) build

# Each selected test directory now appears as a first-class prerequisite.
$(BUILD_TARGETS): | $(SHARED_BOOTSTRAP_TARGET)
	+$(MAKE) -C $(@D) build

.PHONY: install
install:
	@$(INSTALL_DIR) $(DESTDIR)$(xtfdir)
	$(INSTALL_PROGRAM) xtf-runner $(DESTDIR)$(xtfdir)
	@find xtf -path '*/__pycache__' -prune -o -name '*.py' -print | \
		while read -r f; do \
			d="$(DESTDIR)$(xtfdir)/$$(dirname "$$f")"; \
			$(INSTALL_DIR) "$$d"; \
			$(INSTALL_DATA) "$$f" "$$d"; \
		done

install: $(SHARED_BOOTSTRAP_TARGET) $(INSTALL_TARGETS)

$(INSTALL_TARGETS): | $(SHARED_BOOTSTRAP_TARGET)
	+$(MAKE) -C $(@D) install

endif

define all_sources
	find include/ arch/ common/ tests/ -name "*.[hcsS]"
endef

.PHONY: cscope
cscope:
	$(all_sources) > cscope.files
	cscope -b -q -k

NINJA_CONTEXT_HASH := $(shell \
	printf '%s\n' \
		'$(sort $(TESTS))' \
		'$(CC)' \
		'$(CPP)' \
		'$(LD)' \
		'$(OBJCOPY)' \
		'$(PYTHON)' \
		'$(LLVM)' \
		'$(CROSS_COMPILE)' \
		| sha1sum | cut -d' ' -f1)
NINJA_CONTEXT_STAMP := $(ROOT)/build/.xtf.ninja.$(NINJA_CONTEXT_HASH).context
NINJA_VARS_FILE := $(ROOT)/build/xtf.ninja.vars
NINJA_FILE := $(ROOT)/build/xtf.ninja
HVM64_FORMAT := $(firstword \
	$(filter elf32-x86-64,$(shell $(OBJCOPY) --help)) \
	elf32-i386)
NINJA_METADATA_INPUTS := \
	$(ROOT)/Makefile \
	$(ROOT)/build/common.mk \
	$(ROOT)/build/core.mk \
	$(ROOT)/build/files.mk \
	$(ROOT)/build/gen.mk \
	$(ROOT)/build/load-tests.mk \
	$(ROOT)/build/gen-ninja.py \
	$(TEST_MAKEFILES) \
	$(wildcard $(ROOT)/Makefile.local)

.PHONY: ninja-vars ninja-file ninja-build ninja-install

ninja-vars:
	@printf 'global\t%s\t%s\n' ROOT '$(ROOT)'
	@printf 'global\t%s\t%s\n' CC '$(CC)'
	@printf 'global\t%s\t%s\n' CPP '$(CPP)'
	@printf 'global\t%s\t%s\n' LD '$(LD)'
	@printf 'global\t%s\t%s\n' OBJCOPY '$(OBJCOPY)'
	@printf 'global\t%s\t%s\n' PYTHON '$(PYTHON)'
	@printf 'global\t%s\t%s\n' DESTDIR '$(DESTDIR)'
	@printf 'global\t%s\t%s\n' xtfdir '$(xtfdir)'
	@printf 'global\t%s\t%s\n' xtftestdir '$(xtftestdir)'
	@printf 'global\t%s\t%s\n' INSTALL '$(INSTALL)'
	@printf 'global\t%s\t%s\n' INSTALL_DATA '$(INSTALL_DATA)'
	@printf 'global\t%s\t%s\n' INSTALL_DIR '$(INSTALL_DIR)'
	@printf 'global\t%s\t%s\n' INSTALL_PROGRAM '$(INSTALL_PROGRAM)'
	@printf 'global\t%s\t%s\n' HVM64_FORMAT '$(HVM64_FORMAT)'
	@printf 'objects\t%s\t%s\n' perbits '$(obj-perbits)'
	@printf 'objects\t%s\t%s\n' perenv '$(obj-perenv)'
	@$(foreach env,$(ALL_ENVIRONMENTS), \
		printf 'env\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
			'$(env)' \
			'$($(env)_guest)' \
			'$($(env)_arch)' \
			'$(AFLAGS_$($(env)_arch))' \
			'$(CFLAGS_$($(env)_arch))' \
			'$(AFLAGS_$(env))' \
			'$(CFLAGS_$(env))' \
			'$(link-$(env))' \
			'$(LDFLAGS_$(env))' \
			'$(defcfg-$($(env)_guest))'; \
	)
	@$(foreach env,$(ALL_ENVIRONMENTS), \
		printf 'env_objects\t%s\t%s\n' \
			'$(env)' \
			'$(obj-$(env))'; \
	)
	@$(foreach key,$(REGISTERED_TESTS), \
		printf 'test\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
			'$(key)' \
			'$(TEST_DIR_$(key))' \
			'$(TEST_NAME_$(key))' \
			'$(TEST_CATEGORY_$(key))' \
			'$(TEST_ENVS_$(key))' \
			'$(TEST_EXTRA_CFG_$(key))' \
			'$(TEST_VARY_CFG_$(key))' \
			'$(TEST_VCPUS_$(key))' \
			'$(TEST_LOCAL_OBJ_PERENV_$(key))'; \
	)

$(NINJA_CONTEXT_STAMP):
	@mkdir -p $(dir $@)
	@: > $@

$(NINJA_VARS_FILE): $(NINJA_CONTEXT_STAMP) $(NINJA_METADATA_INPUTS)
	@$(MAKE) -s ninja-vars TESTS='$(TESTS)' > $@.tmp
	@if ! cmp -s $@.tmp $@ 2>/dev/null; then mv -f $@.tmp $@; else rm -f $@.tmp; fi

$(NINJA_FILE): $(NINJA_VARS_FILE) $(ROOT)/build/gen-ninja.py
	@cd $(ROOT) && $(PYTHON) build/gen-ninja.py $(NINJA_VARS_FILE) $@.tmp
	@if ! cmp -s $@.tmp $@ 2>/dev/null; then mv -f $@.tmp $@; else rm -f $@.tmp; fi

ninja-file: $(NINJA_FILE)

ninja-build: ninja-file
	cd $(ROOT) && ninja -f $(NINJA_FILE)

ninja-install: ninja-file
	cd $(ROOT) && ninja -f $(NINJA_FILE) install

.PHONY: gtags
gtags:
	$(all_sources) | gtags -f -

.PHONY: clean
clean:
	find . \( -name "*.o" -o -name "*.d" -o -name "*.lds" \) -delete
	find tests/ \( -perm -a=x -name "test-*" -o -name "test-*.cfg" \
		-o -name "info.json" \) -delete
	rm -f $(ROOT)/build/{.,}xtf.ninja* .ninja_deps .ninja_log \
		$(NINJA_VARS_FILE) $(NINJA_FILE)

.PHONY: distclean
distclean: clean
	find . \( -name "*~" -o -name "cscope*" \) -delete
	rm -rf docs/autogenerated/ dist/

.PHONY: doxygen
doxygen: Doxyfile
	doxygen Doxyfile > /dev/null

.PHONY: pylint
pylint:
	-pylint --rcfile=.pylintrc xtf-runner xtf

.PHONY: runner-selftest
runner-selftest:
	$(PYTHON) -m xtf.runner.selftest
