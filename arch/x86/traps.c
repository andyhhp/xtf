#include <xtf/lib.h>
#include <xtf/traps.h>

#include <arch/x86/processor.h>

/*
 * Evaluate whether this exception is a trap or an interrupt.  i.e. whether it
 * save to just return at the current %eip, or whether further action is
 * required to resolve the source of the exception.
 */
static bool is_trap_or_interrupt(const struct cpu_regs *regs)
{
    /* All non-reserved vectors are interrupts. */
    if ( regs->entry_vector >= X86_NR_RESERVED_VECTORS )
        return true;

    uint32_t vec_bit = 1u << regs->entry_vector;

    if ( vec_bit & (X86_EXC_TRAPS | X86_EXC_INTERRUPTS) )
        return true;

    /* Architectural faults and aborts all need further action. */
    if ( vec_bit & (X86_EXC_FAULTS | X86_EXC_ABORTS) )
        return false;

    /*
     * The Debug Exception is awkward, and either a trap or a fault depending
     * on other conditions.
     */
    unsigned long dr6 = read_dr6();

    /* General Detect is a fault. */
    if ( dr6 & X86_DR6_BD )
        return false;

    /* Instruction breakpoints are faults. */
    if ( dr6 & (X86_DR6_B0 | X86_DR6_B1 | X86_DR6_B2 | X86_DR6_B3) )
    {
        unsigned long dr7 = read_dr7(), bp;

        for ( bp = 0; bp < 4; ++bp )
        {
            if ( (dr6 & (1u << bp)) &&              /* Breakpoint triggered? */
                 ((dr7 & (3u << (bp + 16))) == 0) ) /* Instruction breakpoint? */
                return false;
        }
    }

    /* All other debug conditions are traps. */
    return true;
}

/*
 * C entry-point for exceptions, after the per-environment stubs have suitably
 * adjusted the stack.
 */
void do_exception(struct cpu_regs *regs)
{
    unsigned long fixup_addr;
    bool safe = is_trap_or_interrupt(regs);

    /* Look in the exception table to see if a redirection has been set up. */
    if ( !safe && (fixup_addr = search_extable(regs->ip)) )
    {
        regs->ip = fixup_addr;
        safe = true;
    }

    if ( !safe )
        panic("Unhandled exception: vec %u at %04x:%p\n",
              regs->entry_vector, regs->cs, _p(regs->ip));
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
