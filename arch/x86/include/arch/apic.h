/**
 * @file arch/x86/include/arch/apic.h
 *
 * %x86 Local APIC register definitions and utility functions.
 *
 * The xAPIC MMIO window is expected to be in its default location for the
 * benefit of unpaged environments.
 */

#ifndef XTF_X86_APIC_H
#define XTF_X86_APIC_H

#include <xtf/types.h>
#include <xen/errno.h>

#include <arch/msr-index.h>

/* Local APIC register definitions. */
#define APIC_ID         0x020
#define APIC_LVR        0x030
#define APIC_SPIV       0x0f0
#define   APIC_SPIV_APIC_ENABLED  0x00100

#define APIC_ICR        0x300
#define   APIC_DM_NMI             0x00400
#define   APIC_ICR_BUSY           0x01000
#define   APIC_DEST_SELF          0x40000

#define APIC_ICR2       0x310

#define APIC_DEFAULT_BASE 0xfee00000ul

/* Utilities. */

enum apic_mode {
    APIC_MODE_UNKNOWN,
    APIC_MODE_NONE,
    APIC_MODE_DISABLED,
    APIC_MODE_XAPIC,
    APIC_MODE_X2APIC,
};

/**
 * Discover and initialise the local APIC to the requested mode.  May fail if
 * there is no APIC, or the requested mode is not available.
 */
int apic_init(enum apic_mode mode);

static inline uint32_t apic_mmio_read(unsigned int reg)
{
    return *(volatile uint32_t *)(_p(APIC_DEFAULT_BASE) + reg);
}

static inline void apic_mmio_write(unsigned int reg, uint32_t val)
{
    *(volatile uint32_t *)(_p(APIC_DEFAULT_BASE) + reg) = val;
}

static inline void apic_mmio_icr_write(uint64_t val)
{
    apic_mmio_write(APIC_ICR2, (uint32_t)(val >> 32));
    apic_mmio_write(APIC_ICR,  (uint32_t)val);
}

static inline uint32_t apic_msr_read(unsigned int reg)
{
    unsigned long val;

    asm volatile ("rdmsr" : "=a" (val)
                  : "c" (MSR_X2APIC_REGS + (reg >> 4)) : "edx");

    return val;
}

static inline void apic_msr_write(unsigned int reg, uint32_t val)
{
    asm volatile ("wrmsr" ::
                  "a" (val), "d" (0),
                  "c" (MSR_X2APIC_REGS + (reg >> 4)));
}

static inline void apic_msr_icr_write(uint64_t val)
{
    asm volatile ("wrmsr" ::
                  "a" ((uint32_t)val), "d" ((uint32_t)(val >> 32)),
                  "c" (MSR_X2APIC_REGS + (APIC_ICR >> 4)));
}

extern enum apic_mode cur_apic_mode;

static inline uint32_t apic_read(unsigned int reg)
{
    if ( cur_apic_mode == APIC_MODE_XAPIC )
        return apic_mmio_read(reg);
    else
        return apic_msr_read(reg);
}

static inline void apic_write(unsigned int reg, uint32_t val)
{
    if ( cur_apic_mode == APIC_MODE_XAPIC )
        return apic_mmio_write(reg, val);
    else
        return apic_msr_write(reg, val);
}

static inline void apic_icr_write(uint64_t val)
{
    if ( cur_apic_mode == APIC_MODE_XAPIC )
        return apic_mmio_icr_write(val);
    else
        return apic_msr_icr_write(val);
}

#endif /* XTF_X86_APIC_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
