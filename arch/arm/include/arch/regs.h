/**
 * @file arch/arm/include/arch/regs.h
 *
 * arm CPU user registers.
 */
#ifndef XTF_ARM_REGS_H
#define XTF_ARM_REGS_H

#ifdef CONFIG_ARM_64
#include <arch/arm64/regs.h>
#else
#include <arch/arm32/regs.h>
#endif

#endif /* XTF_ARM_REGS_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
