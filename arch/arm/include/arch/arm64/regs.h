/**
 * @file arch/arm/include/arch/arm64/regs.h
 *
 * arm64 CPU user registers.
 */
#ifndef XTF_ARM64_REGS_H
#define XTF_ARM64_REGS_H

#include <xtf/types.h>

#ifndef __ASSEMBLY__
/* Anonymous union includes both 32- and 64-bit names (e.g., r0/x0). */
#define __DECL_REG(n64, n32) union {            \
    uint64_t n64;                               \
    uint32_t n32;                               \
}

struct cpu_regs
{
    /*
     * The mapping AArch64 <-> AArch32 is based on D1.20.1 in ARM DDI
     * 0487A.d.
     *
     *         AArch64       AArch32
     */
    __DECL_REG(x0,           r0);
    __DECL_REG(x1,           r1);
    __DECL_REG(x2,           r2);
    __DECL_REG(x3,           r3);
    __DECL_REG(x4,           r4);
    __DECL_REG(x5,           r5);
    __DECL_REG(x6,           r6);
    __DECL_REG(x7,           r7);
    __DECL_REG(x8,           r8);
    __DECL_REG(x9,           r9);
    __DECL_REG(x10,          r10);
    __DECL_REG(x11 ,         r11);
    __DECL_REG(x12,          r12);

    __DECL_REG(x13,          sp_usr);
    __DECL_REG(x14,          lr_usr);

    __DECL_REG(x15,          __unused_sp_hyp);

    __DECL_REG(x16,          lr_irq);
    __DECL_REG(x17,          sp_irq);

    __DECL_REG(x18,          lr_svc);
    __DECL_REG(x19,          sp_svc);

    __DECL_REG(x20,          lr_abt);
    __DECL_REG(x21,          sp_abt);

    __DECL_REG(x22,          lr_und);
    __DECL_REG(x23,          sp_und);

    __DECL_REG(x24,          r8_fiq);
    __DECL_REG(x25,          r9_fiq);
    __DECL_REG(x26,          r10_fiq);
    __DECL_REG(x27,          r11_fiq);
    __DECL_REG(x28,          r12_fiq);
    __DECL_REG(/* x29 */ fp, /* r13_fiq */ sp_fiq);

    __DECL_REG(/* x30 */ lr, /* r14_fiq */ lr_fiq);

    uint64_t sp;

    /* Return address and mode */
    __DECL_REG(pc,           pc32);
    uint64_t cpsr;
    uint64_t hsr;

    /* The kernel frame should be 16-byte aligned. */
    uint64_t pad0;

    union {
        uint64_t spsr_el1;       /* AArch64 */
        uint64_t spsr_svc;       /* AArch32 */
    };

    /* AArch32 guests only */
    uint32_t spsr_fiq, spsr_irq, spsr_und, spsr_abt;

    /* AArch64 guests only */
    uint64_t sp_el0;
    uint64_t sp_el1, elr_el1;
};
#endif /* __ASSEMBLY__ */

#endif /* XTF_ARM64_REGS_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
