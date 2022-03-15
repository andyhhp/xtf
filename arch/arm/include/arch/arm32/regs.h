/**
 * @file arch/arm/include/arch/arm32/regs.h
 *
 * arm32 CPU user registers.
 */
#ifndef XTF_ARM32_REGS_H
#define XTF_ARM32_REGS_H

#include <xtf/types.h>

#ifndef __ASSEMBLY__
struct cpu_regs
{
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    union {
        uint32_t r11;
        uint32_t fp;
    };
    uint32_t r12;
    uint32_t sp;

    union {
        uint32_t lr;
        uint32_t lr_usr;
    };

    union {
        uint32_t pc, pc32;
    };

    uint32_t cpsr;
    uint32_t hsr;

    uint32_t sp_usr;

    uint32_t sp_irq, lr_irq;
    uint32_t sp_svc, lr_svc;
    uint32_t sp_abt, lr_abt;
    uint32_t sp_und, lr_und;

    uint32_t r8_fiq, r9_fiq, r10_fiq, r11_fiq, r12_fiq;
    uint32_t sp_fiq, lr_fiq;

    uint32_t spsr_svc, spsr_abt, spsr_und, spsr_irq, spsr_fiq;

    /* The stack should be 8-byte aligned */
    uint32_t pad1;
};
#endif /* __ASSEMBLY__ */

#endif /* XTF_ARM32_REGS_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
