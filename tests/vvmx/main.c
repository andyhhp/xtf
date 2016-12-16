/**
 * @file tests/vvmx/main.c
 * @ref test-vvmx
 *
 * @page test-vvmx vvmx
 *
 * Functional testing of the VMX features in a nested-virt environment.
 *
 * @see tests/vvmx/main.c
 */
#include "test.h"

const char test_title[] = "Nested VT-x testing";

void test_main(void)
{
    if ( !cpu_has_vmx )
        return xtf_skip("Skip: VT-x not available\n");

    if ( !vendor_is_intel )
        xtf_warning("Warning: VT-x found on non-Intel processor\n");

    test_msr_vmx();

    if ( xtf_status_reported() )
        return; /* No point continuing if the basic MSRs aren't working. */

    vmx_collect_data();

    test_vmxon();

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
