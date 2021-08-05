/**
 * @file arch/arm/include/arch/system.h
 */
#ifndef XTF_ARM_SYSTEM_H
#define XTF_ARM_SYSTEM_H

#ifdef CONFIG_ARM_64
#include <arch/arm64/system.h>
#else
#include <arch/arm32/system.h>
#endif

#endif /* XTF_ARM_SYSTEM_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
