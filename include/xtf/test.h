/**
 * @file include/xtf/test.h
 *
 * API for tests.
 */
#ifndef XTF_TEST_H
#define XTF_TEST_H

#include <xtf/types.h>

/**
 * To be implemented by each test, as its entry point.
 */
void test_main(void);

/**
 * The title of the test.
 */
extern const char test_title[];

/**
 * Boolean indicating whether generic Force Emulation Prefix support is
 * available for the test to use.
 */
extern bool xtf_has_fep;

/**
 * Boolean indicating whether the test wants user mappings or not.
 *
 * Some tests want an easy transition between supervisor and user modes.  For
 * such tests, this requires running on _PAGE_USER mappings, and they must
 * opt-in.
 *
 * @see @ref errata
 *
 * The framework variable is a weak reference, and may be overridden by a test
 * wishing to change the default.
 */
extern bool test_wants_user_mappings;

/**
 * Boolean indicating whether the test is entirely predicated on the available
 * of the Force Emulation Prefix.
 */
extern bool test_needs_fep;

#endif /* XTF_TEST_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
