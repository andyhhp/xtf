/**
 * @file arch/x86/include/arch/x86-dbg-reg.h
 *
 * %x86 Debug Register Infrastructure
 */

#ifndef XTF_X86_DBG_REG_H
#define XTF_X86_DBG_REG_H

#include <xtf/macro_magic.h>

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
#define X86_DR6_NOT_RTM         (1u << 16)  /* #DB/#BP in RTM region   */

#define X86_DR6_RESET           0xffff0ff0u

/*
 * DR7 unique control bits.
 */
#define X86_DR7_LE              (1u <<  8)  /* Local Exact Breakpoints  */
#define X86_DR7_GE              (1u <<  9)  /* Global Exact Breakpoints */
#define X86_DR7_RTM             (1u << 11)  /* Debugging in RTM regions */
#define X86_DR7_GD              (1u << 13)  /* General Detect           */

/*
 * DR7 common control bits.  Intended for use with the DR7_SYM() helper.
 */
#define X86_DR7_0_L             (1u <<  0)
#define X86_DR7_1_L             (1u <<  2)
#define X86_DR7_2_L             (1u <<  4)
#define X86_DR7_3_L             (1u <<  6)

#define X86_DR7_0_G             (1u <<  1)
#define X86_DR7_1_G             (1u <<  3)
#define X86_DR7_2_G             (1u <<  5)
#define X86_DR7_3_G             (1u <<  7)

#define X86_DR7_0_X             (0u << 16)
#define X86_DR7_0_W             (1u << 16)
#define X86_DR7_0_IO            (2u << 16)
#define X86_DR7_0_RW            (3u << 16)

#define X86_DR7_0_8             (0u << 18)
#define X86_DR7_0_16            (1u << 18)
#define X86_DR7_0_64            (2u << 18)
#define X86_DR7_0_32            (3u << 18)

#define X86_DR7_1_X             (0u << 20)
#define X86_DR7_1_W             (1u << 20)
#define X86_DR7_1_IO            (2u << 20)
#define X86_DR7_1_RW            (3u << 20)

#define X86_DR7_1_8             (0u << 22)
#define X86_DR7_1_16            (1u << 22)
#define X86_DR7_1_64            (2u << 22)
#define X86_DR7_1_32            (3u << 22)

#define X86_DR7_2_X             (0u << 24)
#define X86_DR7_2_W             (1u << 24)
#define X86_DR7_2_IO            (2u << 24)
#define X86_DR7_2_RW            (3u << 24)

#define X86_DR7_2_8             (0u << 26)
#define X86_DR7_2_16            (1u << 26)
#define X86_DR7_2_64            (2u << 26)
#define X86_DR7_2_32            (3u << 26)

#define X86_DR7_3_X             (0u << 28)
#define X86_DR7_3_W             (1u << 28)
#define X86_DR7_3_IO            (2u << 28)
#define X86_DR7_3_RW            (3u << 28)

#define X86_DR7_3_8             (0u << 30)
#define X86_DR7_3_16            (1u << 30)
#define X86_DR7_3_64            (2u << 30)
#define X86_DR7_3_32            (3u << 30)

/**
 * Create a partial @%dr7 setting for a particular breakpoint based on
 * mnemonics.
 *
 * @param bp Breakpoint.  Must be in the range 0 ... 3
 * @param ... Partial X86_DR7_ tokens
 *
 * Example usage:
 * - DR7_SYM(1, G, RW, 8) => Breakpoint 1, global read/write 8-bit
 * - DR7_SYM(3, L, X) => Breakpoint 3, local instruction
 */
#define DR7_SYM(bp, ...) TOK_OR(X86_DR7_ ## bp ## _, ##__VA_ARGS__)

static inline void write_dr0(unsigned long linear)
{
    asm volatile ("mov %0, %%dr0" :: "r" (linear));
}

static inline void write_dr1(unsigned long linear)
{
    asm volatile ("mov %0, %%dr1" :: "r" (linear));
}

static inline void write_dr2(unsigned long linear)
{
    asm volatile ("mov %0, %%dr2" :: "r" (linear));
}

static inline void write_dr3(unsigned long linear)
{
    asm volatile ("mov %0, %%dr3" :: "r" (linear));
}

static inline unsigned long read_dr6(void)
{
    unsigned long val;

    asm volatile ("mov %%dr6, %0" : "=r" (val));

    return val;
}

static inline void write_dr6(unsigned long val)
{
    asm volatile ("mov %0, %%dr6" :: "r" (val));
}

static inline unsigned long read_dr7(void)
{
    unsigned long val;

    asm volatile ("mov %%dr7, %0" : "=r" (val));

    return val;
}

static inline void write_dr7(unsigned long val)
{
    asm volatile ("mov %0, %%dr7" :: "r" (val));
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
