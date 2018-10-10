/**
 * @file tests/xsa-279/main.c
 * @ref test-xsa-279
 *
 * @page test-xsa-279 XSA-279
 *
 * Advisory: [XSA-279](http://xenbits.xen.org/xsa/advisory-279.html)
 *
 * When `PCID` support was added to Xen to mitigate some of the performance
 * hit from the Meltdown protection, Xen's internal TLB flushing changed from
 * using `INVLPG` to using `INVPCID`.  These instructions differ in how they
 * handle non-canonical addresses, with the latter raising a @#GP[0] fault.
 *
 * One path passed an un-audited guest value into Xen's internal TLB flushing
 * logic, which used to be safe, but no longer is.
 *
 * Deliberately try to invalidate a non-canonical address.  If Xen is using
 * the INVPCID instruction for TLB management (at the time of writing, Haswell
 * and later hardware, PCID or INVPCID not explicitly disabled on the command
 * line, and XPTI activated for the domain), and is vulnerable, it will die
 * with an unhandled @#GP[0] fault.
 *
 * @see tests/xsa-279/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-279 PoC";

void test_main(void)
{
    hypercall_update_va_mapping(1ul << 63, 0, UVMF_INVLPG | UVMF_ALL);

    xtf_success("Success: Not vulnerable to XSA-279\n");
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
