/**
 * @file common/setup.c
 *
 * C entry point.
 */

#include <xtf/test.h>

/**
 * Entry point into C.
 *
 * Set up the microkernel and invoke the test.
 */
void xtf_main(void)
{
    test_main();
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
