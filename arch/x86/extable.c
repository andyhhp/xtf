/**
 * @file arch/x86/extable.c
 *
 * Common x86 exception table helper functions.
 */
#include <xtf/lib.h>
#include <xtf/extable.h>

#include <arch/x86/regs.h>

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
    regs->ax = (uint32_t)(regs->entry_vector << 16) | regs->error_code;
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
