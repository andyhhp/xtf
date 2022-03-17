/**
 * @file arch/arm/include/arch/barrier.h
 *
 * arm memory barriers.
 */
#ifndef XTF_ARM_BARRIER_H
#define XTF_ARM_BARRIER_H

#include <xtf/compiler.h>

#define isb()           __asm__ __volatile__ ("isb" : : : "memory")
#define dsb(scope)      __asm__ __volatile__ ("dsb " #scope : : : "memory")
#define dmb(scope)      __asm__ __volatile__ ("dmb " #scope : : : "memory")

#define mb()            dsb(sy)
#ifdef CONFIG_ARM_64
#define rmb()           dsb(ld)
#else
#define rmb()           dsb(sy)
#endif
#define wmb()           dsb(st)

#define smp_mb()        dmb(ish)
#ifdef CONFIG_ARM_64
#define smp_rmb()       dmb(ishld)
#else
#define smp_rmb()       dmb(ish)
#endif
#define smp_wmb()       dmb(ishst)

#endif /* XTF_ARM_BARRIER_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
