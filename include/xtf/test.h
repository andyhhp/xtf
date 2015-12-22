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
 * Boolean indicating whether generic Force Emulation Prefix support is
 * available for the test to use.
 */
extern const bool xtf_has_fep;

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
