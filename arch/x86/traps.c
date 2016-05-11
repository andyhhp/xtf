#include <xtf/lib.h>
#include <xtf/traps.h>
#include <xtf/exlog.h>

#include <arch/x86/decode.h>
#include <arch/x86/lib.h>
#include <arch/x86/processor.h>

bool (*xtf_unhandled_exception_hook)(struct cpu_regs *regs);

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
    const struct extable_entry *ex;
    bool safe = is_trap_or_interrupt(regs);

    xtf_exlog_log_exception(regs);

    /* Look in the exception table to see if a redirection has been set up. */
    if ( !safe && (ex = search_extable(regs->ip)) )
    {
        if ( ex->handler )
            safe = ex->handler(regs, ex);
        else
        {
            regs->ip = ex->fixup;
            safe = true;
        }
    }

    /*
     * If the test has installed an unhandled exception hook, call it in the
     * hope that it can resolve the exception.
     */
    if ( !safe && xtf_unhandled_exception_hook )
        safe = xtf_unhandled_exception_hook(regs);

    /* Still unresolved? Give up and panic() with some relevent information. */
    if ( !safe )
    {
        char buf[16];

        x86_exc_decode_ec(buf, ARRAY_SIZE(buf),
                          regs->entry_vector, regs->error_code);

        if ( regs->entry_vector == X86_EXC_PF )
        {
            unsigned long cr2 = read_cr2();

            panic("Unhandled exception at %04x:%p\n"
                  "Vec %u %s[%s] %%cr2 %p\n",
                  regs->cs, _p(regs->ip), regs->entry_vector,
                  x86_exc_short_name(regs->entry_vector), buf, _p(cr2));
        }
        else
            panic("Unhandled exception at %04x:%p\n"
                  "Vec %u %s[%s]\n",
                  regs->cs, _p(regs->ip), regs->entry_vector,
                  x86_exc_short_name(regs->entry_vector), buf);
    }
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
