/**
 * @file tests/debug-regs/main.c
 * @ref test-debug-regs
 *
 * @page test-debug-regs Debug register and control tests
 *
 * The following misc tests are implemented:
 *
 * 1.  Xen, between
 *     [65e3554908](http://xenbits.xen.org/gitweb/?p=xen.git;a=commitdiff;h=65e355490817ac1783c9ef06c13cf980edf05b5b)
 *     (Introduced in Xen 4.5) and
 *     [adf8feba1a](http://xenbits.xen.org/gitweb/?p=xen.git;a=commitdiff;h=adf8feba1afa040f3a84a82953e18af02060884a)
 *     (Fixed in Xen 4.11) had a bug whereby some writes to @%dr7 didn't take
 *     immediate effect.
 *
 *     In practice, this renders guest debugging useless, as the guest kernels
 *     context switch of @%dr7 fails to take effect until the next full vcpu
 *     reschedule.
 *
 * @see tests/debug-regs/main.c
 */
#include <xtf.h>

const char test_title[] = "Debugging facility tests";

/*
 * Attempt to detect a failure to latch %dr7.  A full vcpu context switch, or
 * a second write to %dr7 will latch the correct value even in the presence of
 * the bug.
 */
static void test_pv_dr7_latch(void)
{
    unsigned int dummy, i;
    unsigned long dr7;

    printk("Test PV %%dr7 latch\n");

    /* Reset any latched %dr7 content. */
    write_dr7(0);
    write_dr7(0);

    /* Point %dr0 at dummy, %dr7 set with %dr0 enabled. */
    write_dr0(_u(&dummy));
    dr7 = X86_DR7_GE | DR7_SYM(0, G, RW, 32);

    /*
     * We can race with the Xen scheduler, which may mask the latching bug.
     * Repeat 10 times, or until we positively see buggy behaviour.
     */
    for ( i = 0; i < 10; ++i )
    {
        exinfo_t fault = 0;

        asm volatile ("mov %[dr7], %%dr7;"
                      "movl $0, %[ptr]; 1:"
                      _ASM_EXTABLE_HANDLER(1b, 1b, ex_record_fault_eax)
                      : "+a" (fault),
                        [ptr] "=m" (dummy)
                      : [dr7] "r" (dr7), "X" (ex_record_fault_eax));

        /* Reset any latched %dr7 content. */
        write_dr7(0);
        write_dr7(0);

        if ( fault == 0 )
        {
            xtf_failure("  Fail: Single write to %%dr7 failed to latch\n");
            break;
        }
        else if ( fault != EXINFO_SYM(DB, 0) )
        {
            xtf_error("  Error: Unexpected fault %#x, %pe\n",
                      fault, _p(fault));
            break;
        }
    }

    if ( i == 10 )
        printk("  All ok - %%dr7 seems to work fine\n");

    /* Reset other state. */
    write_dr0(0);
    write_dr6(X86_DR6_RESET);
}

void test_main(void)
{
    if ( IS_DEFINED(CONFIG_PV) )
        test_pv_dr7_latch();

    xtf_success(NULL);
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
