#include "test.h"

/* vmxon region which shouldn't be latched in the hardware vmxon pointer. */
static uint8_t vmxon_region_unused[PAGE_SIZE] __page_aligned_bss;

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

void test_vmxon(void)
{
    unsigned long cr4 = read_cr4();

    if ( cr4 & X86_CR4_VMXE )
        write_cr4(cr4 &= ~X86_CR4_VMXE);

    printk("Test: vmxon\n");

    test_vmxon_novmxe();
    test_vmxon_novmxe_in_user();

    write_cr4(cr4 |= X86_CR4_VMXE);

    test_vmxon_in_user();
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
