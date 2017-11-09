/**
 * @file tests/lbr-tsx-vmentry/main.c
 * @ref test-lbr-tsx-vmentry
 *
 * @page test-lbr-tsx-vmentry LBR/TSX VMentry failure
 *
 * MSR load and save lists are a VT-x capability whereby hardware logically
 * performs a rdmsr() from the load list on VMentry, and a wrmsr() to the save
 * list on VMexit.  This facility is used by Xen for a number of MSRs,
 * including the LBR registers if the guest enables MSR_DEBUGCTL.LBR.
 *
 * On Haswell and later hardware, the LBR format includes two TSX bits (61 and
 * 62) in the LBR entries.
 *
 * When TSX has been disabled (most usually via microcode), hardware writes
 * LBR records into the MSR load/save area wit the TSX bits cleared.  However
 * with TSX disabled, hardware expects the TSX bits to be part of the linear
 * address in the record (i.e. properly sign extended).
 *
 * Therefore, when an LBR record is generated in the upper canonical region,
 * the value written into the save record by VMexit is not tolerated by the
 * VMentry logic, resulting in a vmentry failure, e.g:
 *
 * <pre>
 *    (XEN) d1v0 vmentry failure (reason 0x80000022): MSR loading (entry 3)
 *    (XEN)   msr 00000680 val 1fff800000102e60 (mbz 0)
 * </pre>
 *
 * In the affected configuration, Xen must fix up the VMentry load list on
 * every VMentry for the VM to continue to function.  This test sets up such a
 * condition, and checks to see whether it keeps running to completion.
 *
 * @see tests/lbr-tsx-vmentry/main.c
 */
#include <xtf.h>

const char test_title[] = "LBR/TSX VMentry failure test";

static void int3_stub(void)
{
    /* Force a LBR record to be generated. */
    asm volatile ("jmp 1f; 1:");

    /*
     * After latching an LBR record, we need to force a subsequent vmentry.
     * Using printk() is the easiest way.
     */
    printk("Latched a Last Branch Record in the upper canonical half\n");
}

void test_main(void)
{
    uint64_t dbgctl;

    /* Alias everything in the upper canonical half. */
    pae_l4_identmap[256] = pae_l4_identmap[0];
    barrier();

    /* Probe for, and enable Last Branch Record. */
    if ( rdmsr_safe(MSR_DEBUGCTL, &dbgctl) )
        return xtf_skip("Skip: MSR_DEBUGCTL not available\n");
    else if ( wrmsr_safe(MSR_DEBUGCTL, dbgctl | MSR_DEBUGCTL_LBR) )
        return xtf_skip("Skip: MSR_DEBUGCTL.LBR not available\n");

    /* Construct a function pointer to int3_stub() via its upper alias. */
    void (*int3_stub_alias)(void) = _p(_u(int3_stub) | 0xffff800000000000);

    int3_stub_alias();

    /*
     * If XTF is alive at this point, the vmentry succeeded.  This will either
     * be unaffected hardware, or Xen has all applicable workarounds.
     */
    xtf_success("Success: No LBR/TSX VMentry failure in this configuration\n");
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
