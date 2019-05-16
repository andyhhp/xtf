#ifndef XTF_TRAPS_H
#define XTF_TRAPS_H

#include <xtf/extable.h>
#include <xtf/test.h>

#include <arch/traps.h>

/**
 * May be implemented by a guest to provide custom exception handling.
 */
bool do_unhandled_exception(struct cpu_regs *regs);

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
