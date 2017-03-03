#ifndef XTF_TRAPS_H
#define XTF_TRAPS_H

#include <xtf/extable.h>

#include <arch/traps.h>

/**
 * Function pointer to allow tests to install an unhandled exception hook.
 *
 * Must only return true if action has been taken resolve the exception.
 * i.e. that it is now safe to iret back.  If not, a panic() will occur.
 */
extern bool (*xtf_unhandled_exception_hook)(struct cpu_regs *regs);

#endif /* XTF_TRAPS_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
