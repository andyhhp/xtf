/**
 * @file arch/x86/extable.c
 *
 * Common x86 exception table helper functions.
 */
#include <xtf/extable.h>

bool ex_record_fault_eax(struct cpu_regs *regs, const struct extable_entry *ex)
{
    regs->ax = EXINFO(regs->entry_vector, regs->error_code);
    regs->ip = ex->fixup;

    return true;
}

bool ex_record_fault_edi(struct cpu_regs *regs, const struct extable_entry *ex)
{
    regs->di = EXINFO(regs->entry_vector, regs->error_code);
    regs->ip = ex->fixup;

    return true;
}

bool ex_rdmsr_safe(struct cpu_regs *regs, const struct extable_entry *ex)
{
    regs->ax = regs->dx = 0;
    regs->cx = -1u;
    regs->ip = ex->fixup;

    return true;
}

bool ex_wrmsr_safe(struct cpu_regs *regs, const struct extable_entry *ex)
{
    regs->cx = -1u;
    regs->ip = ex->fixup;

    return true;
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
