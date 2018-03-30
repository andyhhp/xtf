/**
 * @file arch/x86/include/arch/hpet.h
 *
 * %x86 HPET register definitions and utility functions.
 *
 * NB: assume the HPET is at it's default address.
 */

#ifndef XTF_X86_HPET_H
#define XTF_X86_HPET_H

#include <xtf/numbers.h>
#include <xtf/types.h>

#define HPET_ID                 0x0
#define   HPET_ID_MAX_PERIOD    0x05f5e100

#define HPET_DEFAULT_BASE       0xfed00000

/**
 * Discover and initialise the HPET.  May fail if there is no HPET.
 */
int hpet_init(void);

static inline uint32_t hpet_read32(unsigned int reg)
{
    return *(volatile uint32_t *)(_p(HPET_DEFAULT_BASE) + reg);
}

static inline uint64_t hpet_read64(unsigned int reg)
{
    return *(volatile uint64_t *)(_p(HPET_DEFAULT_BASE) + reg);
}

static inline void hpet_write32(unsigned int reg, uint32_t val)
{
    *(volatile uint32_t *)(_p(HPET_DEFAULT_BASE) + reg) = val;
}

static inline void hpet_write64(unsigned int reg, uint64_t val)
{
    *(volatile uint64_t *)(_p(HPET_DEFAULT_BASE) + reg) = val;
}

#endif /* !XTF_X86_HPET_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
