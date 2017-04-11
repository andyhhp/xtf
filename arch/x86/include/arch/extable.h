/**
 * @file arch/x86/include/arch/extable.h
 *
 * Common x86 exception table helper functions.
 */
#ifndef XTF_X86_EXTABLE_H
#define XTF_X86_EXTABLE_H

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
bool ex_record_fault_eax(struct cpu_regs *regs, const struct extable_entry *ex);

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
bool ex_record_fault_edi(struct cpu_regs *regs, const struct extable_entry *ex);

/**
 * Fixup from a rdmsr fault
 *
 * Clobber the MSR index to signify error, and zero output.
 */
bool ex_rdmsr_safe(struct cpu_regs *regs, const struct extable_entry *ex);

/**
 * Fixup from a wrmsr fault
 *
 * Clobber the MSR index to signify error.
 */
bool ex_wrmsr_safe(struct cpu_regs *regs, const struct extable_entry *ex);

#endif /* XTF_X86_EXTABLE_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
