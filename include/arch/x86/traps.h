#ifndef XTF_X86_TRAPS_H
#define XTF_X86_TRAPS_H

#include <xtf/compiler.h>
#include <arch/x86/regs.h>
#include <arch/x86/page.h>

/*
 * Arch-specific function to initialise the exception entry points, etc.
 */
void arch_init_traps(void);

/*
 * Arch-specific function to quiesce the domain, in the event that a
 * shutdown(crash) hypercall has not succeeded.
 */
void __noreturn arch_crash_hard(void);

extern uint8_t boot_stack[2 * PAGE_SIZE];

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
