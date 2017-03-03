/**
 * @file arch/x86/extable.c
 *
 * Common x86 exception table helper functions.
 */
#include <xtf/lib.h>
#include <xtf/extable.h>

#include <arch/exinfo.h>
#include <arch/regs.h>

/**
 * Record the current fault in @%eax
 *
 * Sample usage:
 * <pre>
 *   asm volatile ("1: $INSN; 2:"
 *                 _ASM_EXTABLE_HANDLER(1b, 2b, ex_record_fault_eax)
 *                 : "=a" (fault) : "0" (0));
 * </pre>
 */
bool ex_record_fault_eax(struct cpu_regs *regs, const struct extable_entry *ex)
{
    regs->ax = EXINFO(regs->entry_vector, regs->error_code);
    regs->ip = ex->fixup;

    return true;
}

/**
 * Record the current fault in @%edi
 *
 * Sample usage:
 * <pre>
 *   asm volatile ("1: $INSN; 2:"
 *                 _ASM_EXTABLE_HANDLER(1b, 2b, ex_record_fault_edi)
 *                 : "=D" (fault) : "0" (0));
 * </pre>
 */
bool ex_record_fault_edi(struct cpu_regs *regs, const struct extable_entry *ex)
{
    regs->di = EXINFO(regs->entry_vector, regs->error_code);
    regs->ip = ex->fixup;

    return true;
}

/**
 * Fixup from a rdmsr fault
 *
 * Clobber the MSR index to signify error, and zero output.
 */
bool ex_rdmsr_safe(struct cpu_regs *regs, const struct extable_entry *ex)
{
    regs->ax = regs->dx = 0;
    regs->cx = -1u;
    regs->ip = ex->fixup;

    return true;
}

/**
 * Fixup from a wrmsr fault
 *
 * Clobber the MSR index to signify error.
 */
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
