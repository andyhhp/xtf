#ifndef XTF_X86_TRAPS_H
#define XTF_X86_TRAPS_H

#include <xtf/compiler.h>
#include <arch/regs.h>
#include <arch/page.h>

/*
 * Arch-specific function to initialise the exception entry points, etc.
 */
void arch_init_traps(void);

/*
 * Arch-specific function to quiesce the domain, in the event that a
 * shutdown(crash) hypercall has not succeeded.
 */
void __noreturn arch_crash_hard(void);

/*
 * Return the correct %ss/%esp from an exception.  In 32bit if no stack switch
 * occurs, an exception frame doesn't contain this information.
 */
unsigned long cpu_regs_sp(const struct cpu_regs *regs);
unsigned int  cpu_regs_ss(const struct cpu_regs *regs);

extern uint8_t boot_stack[3 * PAGE_SIZE];

#if defined(CONFIG_PV)
#include <xen/xen.h>

extern struct start_info *start_info;
#endif

#endif /* XTF_X86_TRAPS_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
