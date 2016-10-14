/**
 * @file tests/cpuid-faulting/main.c
 * @ref test-cpuid-faulting
 *
 * @page test-cpuid-faulting CPUID Faulting support
 *
 * CPUID Faulting is a feature available natively on Intel IvyBridge and newer
 * processors, which allows a kernel or hypervisor to trap userspace `CPUID`
 * instructions.
 *
 * Xen provides this support to guests, and for HVM guests is the the position
 * to offer CPUID Faulting even on hardware which lacks it natively.
 *
 * The native definition of CPUID Faulting is that when enabled, `CPUID`
 * instructions suffer @#GP[0] when executed at CPL > 0.  This behaviour is
 * honoured exactly for HVM guests.  PV guests kernels however execute in ring
 * 1 (32bit PV) or ring 3 (64bit PV).  For PV guests, the implemented
 * behaviour will cause a @#GP[0] fault when executed in guest userspace.
 *
 * @see tests/cpuid-faulting/main.c
 */
#include <xtf.h>

#include <arch/x86/msr-index.h>
#include <arch/x86/processor.h>

bool test_wants_user_mappings = true;

#define EXC_SYM(vec, ec) ((X86_EXC_ ## vec) << 16 | ec)

unsigned long stub_cpuid(void)
{
    unsigned int fault = 0, tmp;

    asm volatile("1: cpuid; 2:"
                 _ASM_EXTABLE_HANDLER(1b, 2b, ex_record_fault_edi)
                 : "=a" (tmp), "+D" (fault)
                 : "a" (0)
                 : "ebx", "ecx", "edx");

    return fault;
}

unsigned long stub_fep_cpuid(void)
{
    unsigned int fault = 0, tmp;

    asm volatile(_ASM_XEN_FEP
                 "1: cpuid; 2:"
                 _ASM_EXTABLE_HANDLER(1b, 2b, ex_record_fault_edi)
                 : "=a" (tmp), "+D" (fault)
                 : "a" (0)
                 : "ebx", "ecx", "edx");

    return fault;
}

static void test_cpuid(bool exp_faulting)
{
    /*
     * Kernel cpuids should never fault
     */
    if ( stub_cpuid() )
        xtf_failure("Fail: kernel cpuid faulted\n");

    if ( IS_DEFINED(CONFIG_PV) && stub_fep_cpuid() )
        xtf_failure("Fail: kernel pv cpuid faulted\n");

    if ( xtf_has_fep && stub_fep_cpuid() )
        xtf_failure("Fail: kernel emulated cpuid faulted\n");

    /*
     * User cpuids should raise #GP[0] if faulting is enabled.
     */
    unsigned long exp = exp_faulting ? EXC_SYM(GP, 0) : 0;
    const char *exp_fail_str = exp_faulting ? "didn't fault" : "faulted";

    if ( exec_user(stub_cpuid) != exp )
        xtf_failure("Fail: user cpuid %s\n", exp_fail_str);

    if ( IS_DEFINED(CONFIG_PV) && exec_user(stub_fep_cpuid) != exp )
        xtf_failure("Fail: user pv cpuid %s\n", exp_fail_str);

    if ( xtf_has_fep && exec_user(stub_fep_cpuid) != exp )
        xtf_failure("Fail: user emulated cpuid %s\n", exp_fail_str);
}

void test_main(void)
{
    uint64_t platform_info, features_enable;

    printk("Guest CPUID Faulting support\n");

    if ( IS_DEFINED(CONFIG_HVM) && !xtf_has_fep )
        xtf_skip("FEP support not detected - some tests will be skipped\n");

    /* No faulting.  CPUID should work without faulting anywhere. */
    printk("Testing CPUID without faulting enabled\n");
    test_cpuid(false);

    /* Probe for CPUID Faulting support. */
    if ( rdmsr_safe(MSR_INTEL_PLATFORM_INFO, &platform_info) ||
         !(platform_info & MSR_PLATFORM_INFO_CPUID_FAULTING) )
        return xtf_skip("Skip: CPUID Faulting unavailable\n");

    if ( rdmsr_safe(MSR_INTEL_MISC_FEATURES_ENABLES, &features_enable) )
        return xtf_error("Error: Fault accessing MISC_FEATURES_ENABLES\n");

    /* Attempt to enable CPUID Faulting. */
    if ( wrmsr_safe(MSR_INTEL_MISC_FEATURES_ENABLES,
                    features_enable | MSR_MISC_FEATURES_CPUID_FAULTING) )
        return xtf_failure("Fail: Unable to enable CPUID Faulting\n");

    /* Faulting active.  CPUID should fault ouside of the kernel. */
    printk("Testing CPUID with faulting enabled\n");
    test_cpuid(true);

    /* Try disabling faulting. */
    if ( wrmsr_safe(MSR_INTEL_MISC_FEATURES_ENABLES, features_enable) )
        return xtf_failure("Fail: Unable to disable CPUID Faulting\n");

    /* Double check that CPUID no longer faults. */
    printk("Retesting CPUID without faulting enabled\n");
    test_cpuid(false);

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
