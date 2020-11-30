/**
 * @file arch/arm/include/arch/traps.h
 */
#ifndef XTF_ARM_TRAPS_H
#define XTF_ARM_TRAPS_H

#include <xtf/compiler.h>
#include <xen/xen.h>

void __noreturn arch_crash_hard(void);

#endif /* XTF_ARM_TRAPS_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
