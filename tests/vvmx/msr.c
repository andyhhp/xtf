#include "test.h"

/*
 * Guest MSR_FEATURE_CONTROL is set by Xen hypervisor instead by guest
 * firmware or hvmloader, so this test checks whether bits in
 * MSR_FEATURE_CONTROL are set correctly and does not require they are all
 * zero.
 */
static void test_msr_feature_control(void)
{
    msr_feature_control_t feat;

    if ( rdmsr_safe(MSR_FEATURE_CONTROL, &feat.raw) )
        return xtf_failure("Fail: Fault when reading MSR_FEATURE_CONTROL\n");

    if ( !cpu_has_smx && feat.vmxon_inside_smx )
        xtf_failure("Fail: FEATURE_CONTROL.VMXON_INSIDE_SMX is set but SMX is not supported\n");

    if ( !feat.vmxon_outside_smx )
        xtf_failure("Fail: FEATURE_CONTROL.VMXON_OUTSIDE_SMX is not set\n");

    /* VMXON should be unusable if LOCK isn't set. */
    if ( !feat.lock )
        xtf_failure("Fail: FEATURE_CONTROL.LOCK is not set\n");

    /* Because LOCK is set, the MSR should be read-only. */
    if ( !wrmsr_safe(MSR_FEATURE_CONTROL, feat.raw) )
        xtf_failure("Fail: Successfully wrote to MSR_FEATURE_CONTROL\n");
}

void test_msr_vmx(void)
{
    printk("Test: MSRs\n");

    test_msr_feature_control();
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
