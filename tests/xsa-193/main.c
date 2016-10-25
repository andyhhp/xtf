/**
 * @file tests/xsa-193/main.c
 * @ref test-xsa-193
 *
 * @page test-xsa-193 XSA-193
 *
 * Advisory: [XSA-193](http://xenbits.xen.org/xsa/advisory-xsa-193.html)
 *
 * Xen change [c42494acb2](http://xenbits.xen.org/gitweb/
 * ?p=xen.git;a=commitdiff;h=c42494acb2f7f31e561d38f06c59a50ee4198f36)
 * switched wrmsr_safe() for wr{f,g}sbase(), neglecting to consider that they
 * internally may use plain wrmsr() or the `wr{f,g}sbase` instructions, both
 * of which will suffer a @#GP fault in this case for non-canonical addresses.
 *
 * Check that Xen properly bounces the @#GP faults back to us, rather than
 * dying itself.
 *
 * @see tests/xsa-193/main.c
 */
#include <xtf.h>

#include <arch/x86/msr-index.h>

const char test_title[] = "XSA-193 PoC";

void test_main(void)
{
    if ( !wrmsr_safe(MSR_FS_BASE, 0x8000000000000000ull) )
        xtf_failure("Fail: MSR_FS_BASE didn't fault for non-canonical value\n");

    if ( !wrmsr_safe(MSR_GS_BASE, 0x8000000000000000ull) )
        xtf_failure("Fail: MSR_GS_BASE didn't fault for non-canonical value\n");

    if ( !wrmsr_safe(MSR_SHADOW_GS_BASE, 0x8000000000000000ull) )
        xtf_failure("Fail: MSR_SHADOW_GS_BASE didn't fault for non-canonical value\n");

    xtf_success("Success: not vulnerable to XSA-193\n");
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
