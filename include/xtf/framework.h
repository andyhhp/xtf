/**
 * @file include/xtf/framework.h
 *
 * Interfaces used by common code, needing to be implemented by
 * arch/environment specific code.  Default stubs are provided in
 * common/weak-defaults.c
 */
#ifndef XTF_FRAMEWORK_H
#define XTF_FRAMEWORK_H

#include <xtf/compiler.h>

/* Set up exception handling, consoles, etc. */
void arch_setup(void);

/* Set up test-specific configuration. */
void test_setup(void);

/*
 * In the case that normal shutdown actions have failed, contain execution as
 * best as possible.
 */
void __noreturn arch_crash_hard(void);

/* Single line summary of execution environment. */
extern const char environment_description[];

#endif /* XTF_FRAMEWORK_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
