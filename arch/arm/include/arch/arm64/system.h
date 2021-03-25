/**
 * @file arch/arm/include/arch/arm64/system.h
 */
#ifndef XTF_ARM64_SYSTEM_H
#define XTF_ARM64_SYSTEM_H

#define local_irq_disable() asm volatile ( "msr daifset, #2\n" ::: "memory" )
#define local_irq_enable()  asm volatile ( "msr daifclr, #2\n" ::: "memory" )

#endif /* XTF_ARM64_SYSTEM_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
