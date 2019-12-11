/**
 * @file tests/splitlock/main.c
 * @ref test-splitlock
 *
 * @page test-splitlock splitlock
 *
 * @todo Docs for test-splitlock
 *
 * @see tests/splitlock/main.c
 */
#include <xtf.h>

const char test_title[] = "Test splitlock";

void test_main(void)
{
    unsigned int *ptr = _p(KB(8) - 2);

    for ( ;; )
        asm volatile ("lock incl %0" : "+m" (*ptr));

    xtf_success(NULL);
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
