/**
 * @file arch/x86/include/arch/x86-dbg-reg.h
 *
 * %x86 Debug Register Infrastructure
 */

#ifndef XTF_X86_DBG_REG_H
#define XTF_X86_DBG_REG_H

/*
 * DR6 status bits.
 */
#define X86_DR6_B0              (1u <<  0)  /* Breakpoint 0 triggered  */
#define X86_DR6_B1              (1u <<  1)  /* Breakpoint 1 triggered  */
#define X86_DR6_B2              (1u <<  2)  /* Breakpoint 2 triggered  */
#define X86_DR6_B3              (1u <<  3)  /* Breakpoint 3 triggered  */
#define X86_DR6_BD              (1u << 13)  /* Debug register accessed */
#define X86_DR6_BS              (1u << 14)  /* Single step             */
#define X86_DR6_BT              (1u << 15)  /* Task switch             */

static inline unsigned long read_dr6(void)
{
    unsigned long val;

    asm volatile ("mov %%dr6, %0" : "=r" (val));

    return val;
}

static inline unsigned long read_dr7(void)
{
    unsigned long val;

    asm volatile ("mov %%dr7, %0" : "=r" (val));

    return val;
}

#endif /* XTF_X86_DBG_REG_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
