/**
 * @file tests/debug-regs/main.c
 * @ref test-debug-regs
 *
 * @page test-debug-regs Debug register and control tests
 *
 * The following PV tests are implemented:
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
 * 2.  Xen, before
 *     [f539ae2706](http://xenbits.xen.org/gitweb/?p=xen.git;a=commitdiff;h=f539ae27061c6811fd5e80e0755bf0514e22b977)
 *     (Xen 4.11) had a bug whereby a write which cleared @%dr7.L/G would
 *     leave stale IO shadow state visible in later reads of @%dr7.
 *
 *     Unfortunately, that changset introduced a second bug, fixed by
 *     [237c31b5a1](http://xenbits.xen.org/gitweb/?p=xen.git;a=commitdiff;h=237c31b5a1d5aa88cdb59b8c31b1b62eb13e82d1)
 *     (Xen 4.11), which caused an attempt to set up an IO breakpoint with
 *     @%cr4.DE clear to clobber an already configured state, despite the
 *     update failing.
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

    /* Reset other state. */
    write_dr0(0);
    write_dr6(X86_DR6_DEFAULT);
}

/*
 * Detect both bugs with shadow IO breakpoint state handling.
 */
static void test_pv_dr7_io_breakpoints(void)
{
    unsigned long io0, io1, dr7, cr4 = read_cr4();

    printk("Test PV %%dr7 IO breakpoints\n");

    if ( !(cr4 & X86_CR4_DE) )
        write_cr4(cr4 |= X86_CR4_DE);

    /* Active IO breakpoint in %dr0. */
    io0 = DR7_SYM(0, G, IO, 32) | X86_DR7_GE | X86_DR7_DEFAULT;
    write_dr7(io0);

    if ( (dr7 = read_dr7()) != io0 )
        xtf_failure("  Fail: dr7 %#lx != io0 %#lx\n",
                    dr7, io0);

    /* Inactive IO breakpoint in %dr1. */
    io1 = DR7_SYM(1, G, IO, 32) | X86_DR7_DEFAULT;
    write_dr7(io1);

    /* Bug 1.  Old %dr0 configuration still visible in %dr7. */
    if ( (dr7 = read_dr7()) != io1 )
        xtf_failure("  Fail: dr7 %#lx != io1 %#lx\n",
                    dr7, io1);

    /* Reload active configuration. */
    write_dr7(io0);

    /* Clear %cr4.de, after which IO breakpoints are invalid. */
    write_cr4(cr4 &= ~X86_CR4_DE);

    /* Attempt to reload an IO breakpoint in %dr0, which should fail ... */
    exinfo_t fault = 0;
    asm volatile ("1: mov %[val], %%dr7; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, ex_record_fault_eax)
                  : "+a" (fault)
                  : [val] "r" (io0),
                    "X" (ex_record_fault_eax));

    if ( fault != EXINFO_SYM(GP, 0) )
        xtf_error("Error: Unexpected fault %pe\n", _p(fault));

    /* Bug 2.  ... but may drop the existing %dr7 configuration. */
    if ( (dr7 = read_dr7()) != io0 )
        xtf_failure("  Fail: dr7 %#lx != io0 %#lx\n",
                    dr7, io0);
}

void test_main(void)
{
    if ( IS_DEFINED(CONFIG_PV) )
    {
        test_pv_dr7_latch();
        test_pv_dr7_io_breakpoints();
    }

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
