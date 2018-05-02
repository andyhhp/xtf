/**
 * @file arch/x86/include/arch/io-apic.h
 *
 * %x86 IO-APIC register definitions and utility functions.
 *
 * The IO-APIC MMIO window is expected to be in its default location for the
 * benefit of unpaged environments.
 */

#ifndef XTF_X86_IO_APIC_H
#define XTF_X86_IO_APIC_H

#include <xtf/numbers.h>
#include <xtf/types.h>

/* MMIO window registers. */
#define IOAPIC_REGSEL             0x00
#define IOAPIC_IOWIN              0x10

/* IO-APIC registers. */
#define IOAPIC_ID                 0x0

#define IOAPIC_VERSION            0x1
#define   IOAPIC_MAXREDIR_MASK    0xff0000

#define IOAPIC_REDIR_ENTRY(e)     (0x10 + (e) * 2)
#define   IOAPIC_REDIR_MASK_SHIFT 16

#define IOAPIC_DEFAULT_BASE       0xfec00000

/**
 * Discover and initialise the IO-APIC.  May fail if there is no IO-APIC.
 */
int ioapic_init(void);

static inline uint32_t ioapic_read32(unsigned int reg)
{
    *(volatile uint32_t *)_p(IOAPIC_DEFAULT_BASE + IOAPIC_REGSEL) = reg;

    return *(volatile uint32_t *)_p(IOAPIC_DEFAULT_BASE + IOAPIC_IOWIN);
}

static inline uint64_t ioapic_read64(unsigned int reg)
{
    return ioapic_read32(reg) | (uint64_t)ioapic_read32(reg + 1) << 32;
}

static inline void ioapic_write32(unsigned int reg, uint32_t val)
{
    *(volatile uint32_t *)_p(IOAPIC_DEFAULT_BASE + IOAPIC_REGSEL) = reg;
    *(volatile uint32_t *)_p(IOAPIC_DEFAULT_BASE + IOAPIC_IOWIN) = val;
}

static inline void ioapic_write64(unsigned int reg, uint64_t val)
{
    ioapic_write32(reg, val);
    ioapic_write32(reg + 1, val >> 32);
}

/**
 * Set the mask bit on a redirection entry.
 */
int ioapic_set_mask(unsigned int entry, bool mask);

#endif /* !XTF_X86_IO_APIC_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
