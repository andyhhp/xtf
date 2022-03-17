/**
 * @file arch/arm/include/arch/arm32/system.h
 */
#ifndef XTF_ARM32_SYSTEM_H
#define XTF_ARM32_SYSTEM_H

#define local_irq_disable() asm volatile ( "cpsid i\n" : : : "cc" )
#define local_irq_enable()  asm volatile ( "cpsie i\n" : : : "cc" )

#endif /* XTF_ARM32_SYSTEM_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
