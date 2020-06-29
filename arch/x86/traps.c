#include <xtf/lib.h>
#include <xtf/traps.h>
#include <xtf/exlog.h>

#include <arch/decode.h>
#include <arch/lib.h>
#include <arch/processor.h>

/*
 * Parameters for fine tuning the exec_user_*() behaviour.  PV guests see the
 * real interrupt flag, so mask it by default.
 */
unsigned long exec_user_cs = __USER_CS;
unsigned long exec_user_ss = __USER_DS;
unsigned long exec_user_efl_and_mask =
    ~(IS_DEFINED(CONFIG_PV) ? X86_EFLAGS_IF : 0);
unsigned long exec_user_efl_or_mask;

/*
 * C entry-point for exceptions, after the per-environment stubs have suitably
 * adjusted the stack.
 */
void do_exception(struct cpu_regs *regs)
{
    const struct extable_entry *ex;
    bool safe = false;

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

    /* Try the unhandled_exception() hook. */
    if ( !safe )
        safe = do_unhandled_exception(regs);

    /* Still unresolved? Give up and panic() with some relevent information. */
    if ( !safe )
    {
        exinfo_t ex = EXINFO(regs->entry_vector, regs->error_code);

        if ( regs->entry_vector == X86_EXC_PF )
        {
            unsigned long cr2 = read_cr2();

            panic("Unhandled exception at %04x:%p\n"
                  "Vec %u %pe %%cr2 %p\n",
                  regs->cs, _p(regs->ip), regs->entry_vector, _p(ex), _p(cr2));
        }
        else
            panic("Unhandled exception at %04x:%p\n"
                  "Vec %u %pe\n",
                  regs->cs, _p(regs->ip), regs->entry_vector, _p(ex));
    }
}

bool __weak do_unhandled_exception(struct cpu_regs *regs)
{
    return false;
}

void __weak do_syscall(struct cpu_regs *regs)
{
    panic("Unhandled syscall\n");
}

void __weak do_sysenter(struct cpu_regs *regs)
{
    panic("Unhandled sysenter\n");
}

void __weak do_evtchn(struct cpu_regs *regs)
{
    panic("Unhandled evtchn upcall\n");
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
