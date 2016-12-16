#include "test.h"

/* vmxon region which shouldn't be latched in the hardware vmxon pointer. */
static uint8_t vmxon_region_unused[PAGE_SIZE] __page_aligned_bss;

/* vmxon region which gets latched in hardware. */
static uint8_t vmxon_region_real[PAGE_SIZE] __page_aligned_bss;

/* Loaded VMCS, to recover VM Instruction Errors. */
static uint8_t vmcs[PAGE_SIZE] __page_aligned_bss;

/**
 * vmxon with CR4.VMXE cleared
 *
 * Expect: @#UD
 */
static void test_vmxon_novmxe(void)
{
    check(__func__, stub_vmxon(0), EXINFO_SYM(UD, 0));
}

/*
 * Wrapper around stub_vmxon_user(), This stub should always fault for control
 * or permission reasons, but pointing at a supervisor frame is useful to
 * check that Xen doesn't dereference the instructions parameter.
 */
static unsigned long __user_text vmxon_in_user(void)
{
    return stub_vmxon_user(_u(vmxon_region_unused));
}

/**
 * vmxon in CPL=3 outside of VMX operation
 *
 * Expect: @#GP(0)
 */
static void test_vmxon_novmxe_in_user(void)
{
    exinfo_t ex = exec_user(vmxon_in_user);

    check(__func__, ex, EXINFO_SYM(UD, 0));
}

/**
 * vmxon in CPL=3 in VMX operation
 *
 * Expect: @#UD
 */
static void test_vmxon_in_user(void)
{
    exinfo_t ex = exec_user(vmxon_in_user);

    check(__func__, ex, EXINFO_SYM(GP, 0));
}

/**
 * vmxon with a physical address that exceeds the maximum address width
 *
 * Expect: VMfailInvalid
 */
static void test_vmxon_overly_wide_paddr(void)
{
    exinfo_t ex = stub_vmxon(1ULL << maxphysaddr);

    check(__func__, ex, VMERR_INVALID);
}

/**
 * vmxon with an unaligned physical address
 *
 * Expect: VMfailInvalid
 */
static void test_vmxon_unaligned_paddr(void)
{
    exinfo_t ex = stub_vmxon(_u(vmxon_region_unused) | 0xff);

    check(__func__, ex, VMERR_INVALID);
}

/**
 * vmxon with VMCS revision ID mismatched with MSR_IA32_VMX_BASIC
 *
 * Expect: VMfailInvalid
 */
static void test_vmxon_mismatched_revid(void)
{
    clear_vmcs(vmxon_region_unused, vmcs_revid ^ 2);
    exinfo_t ex = stub_vmxon(_u(vmxon_region_unused));

    check(__func__, ex, VMERR_INVALID);
}

/**
 * vmxon with VMCS revision ID[31] set
 *
 * Expect: VMfailInvalid
 */
static void test_vmxon_revid_bit31(void)
{
    clear_vmcs(vmxon_region_unused, vmcs_revid | (1UL << 31));
    exinfo_t ex = stub_vmxon(_u(vmxon_region_unused));

    check(__func__, ex, VMERR_INVALID);
}

/**
 * vmxon expected to succeed
 *
 * Expect: Success
 */
static void test_vmxon_correct(void)
{
    clear_vmcs(vmxon_region_real, vmcs_revid);
    exinfo_t ex = stub_vmxon(_u(vmxon_region_real));

    check(__func__, ex, VMERR_SUCCESS);
}

/**
 * vmxon in VMX root w/ CPL = 0 and w/o current VMCS
 *
 * Expect: VMfailInvalid
 */
static void test_vmxon_novmcs_in_root_cpl0(void)
{
    clear_vmcs(vmxon_region_unused, vmcs_revid);
    exinfo_t ex = stub_vmxon(_u(vmxon_region_unused));

    check(__func__, ex, VMERR_INVALID);
}

/**
 * vmxon in VMX root w/ CPL = 3 and w/o current VMCS
 *
 * Expect: @#GP(0)
 */
static void test_vmxon_novmcs_in_root_user(void)
{
    clear_vmcs(vmxon_region_unused, vmcs_revid);
    exinfo_t ex = exec_user(vmxon_in_user);

    check(__func__, ex, EXINFO_SYM(GP, 0));
}

/**
 * vmxon in VMX root w/ CPL = 0 and w/ current VMCS
 *
 * Expect: VMfailvalid()
 */
static void test_vmxon_in_root_cpl0(void)
{
    clear_vmcs(vmxon_region_unused, vmcs_revid);
    exinfo_t ex = stub_vmxon(_u(vmxon_region_unused));

    check(__func__, ex, VMERR_VALID(VMERR_VMXON_IN_ROOT));
}

/**
 * vmxon in VMX root w/ CPL = 3 and w/ current VMCS
 *
 * Expect: @#GP(0)
 */
static void test_vmxon_in_root_user(void)
{
    clear_vmcs(vmxon_region_unused, vmcs_revid);
    exinfo_t ex = exec_user(vmxon_in_user);

    check(__func__, ex, EXINFO_SYM(GP, 0));
}

void test_vmxon(void)
{
    unsigned long cr4 = read_cr4();
    exinfo_t ex;

    if ( cr4 & X86_CR4_VMXE )
        write_cr4(cr4 &= ~X86_CR4_VMXE);

    printk("Test: vmxon\n");

    test_vmxon_novmxe();
    test_vmxon_novmxe_in_user();

    write_cr4(cr4 |= X86_CR4_VMXE);

    test_vmxon_in_user();
    test_vmxon_overly_wide_paddr();
    test_vmxon_unaligned_paddr();
    test_vmxon_mismatched_revid();
    test_vmxon_revid_bit31();
    test_vmxon_correct();

    /* Test should now be operating in VMX Root mode. */

    test_vmxon_novmcs_in_root_cpl0();
    test_vmxon_novmcs_in_root_user();

    /* Load a real VMCS to recover VM Instruction Errors. */
    clear_vmcs(vmcs, vmcs_revid);
    ex = stub_vmptrld(_u(vmcs));
    if ( ex )
        return xtf_failure("Fail: unexpected vmptrld failure %08x\n", ex);

    test_vmxon_in_root_cpl0();
    test_vmxon_in_root_user();
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
