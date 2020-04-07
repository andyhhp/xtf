/**
 * @file tests/xsa-316/main.c
 * @ref test-xsa-316
 *
 * @page test-xsa-316 XSA-316
 *
 * Advisory: [XSA-316](https://xenbits.xen.org/xsa/advisory-316.html)
 *
 * XSA-316 describes an issue with error handling in GNTTABOP_map_grant
 * operation code.
 *
 * Grant table operations are expected to return 0 for success, and a negative
 * number for errors. Some misplaced brackets cause one error path to return 1
 * instead of a negative value.
 *
 * @see tests/xsa-316/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-316 PoC";

static uint8_t frame[PAGE_SIZE] __page_aligned_bss;

void test_main(void)
{
    int rc = xtf_init_grant_table(1);

    if ( rc )
        return xtf_error("Error initialising grant table: %d\n", rc);

    int domid = xtf_get_domid();

    if ( domid < 0 )
        return xtf_error("Error getting domid\n");

    /*
     * Construct gref 8 to allow frame[] to be mapped by ourselves..
     */
    gnttab_v1[8].domid = domid;
    gnttab_v1[8].frame = virt_to_gfn(frame);
    smp_wmb();
    gnttab_v1[8].flags = GTF_permit_access;

    struct gnttab_map_grant_ref map = {
        .host_addr = KB(4),
        .flags = GNTMAP_host_map,
        .ref = 8,
        .dom = 0, /* .. but provide incorrect domain id to map operation. */
    };

    /*
     * Attempt to map gref to exercise the faulty error path.
     */
    rc = hypercall_grant_table_op(GNTTABOP_map_grant_ref, &map, 1);
    if ( map.status > 0 )
        return xtf_failure("Fail: Vulnerable to XSA-316\n");
    else if ( !rc && map.status == GNTST_general_error )
        return xtf_success("Success: Not vulnerable to XSA-316\n");
    else
        return xtf_error("Error: Unexpected result: %d/%d\n", rc, map.status);
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
