/**
 * @file tests/xsa-333/main.c
 * @ref test-xsa-333
 *
 * @page test-xsa-333 XSA-333
 *
 * Advisory: [XSA-333](https://xenbits.xen.org/xsa/advisory-333.html)
 *
 * Xen, after
 * [4fdc932b3cc](https://xenbits.xen.org/gitweb/?p=xen.git;a=commitdiff;h=4fdc932b3cced15d6e73c94ae0192d989fefdc90)
 * (Introduced in Xen 4.11) had no WRMSR-safety for guest accesses to
 * MSR_MISC_ENABLE.  This MSR doesn't exist on AMD hardware, and Xen would
 * crash on access.
 *
 * @see tests/xsa-333/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-333 PoC";

void test_main(void)
{
    unsigned long val;

    rdmsr_safe(MSR_MISC_ENABLE, &val);

    /*
     * If Xen is still alive at this point, it didn't take a #GP fault
     * handling the MSR emulation.
     */

    xtf_success("Success: Not vulnerable to XSA-333\n");
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
