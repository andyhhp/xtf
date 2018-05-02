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
#include <xtf/macro_magic.h>
#include <xtf/types.h>

#define HPET_ID                 0x0
#define   HPET_ID_MAX_PERIOD    0x05f5e100
#define   HPET_ID_NUMBER_MASK   0x1f00

#define HPET_CFG                0x010
#define   HPET_CFG_ENABLE       0x001

#define HPET_COUNTER            0x0f0

#define HPET_Tn_CFG(n)         (0x100 + (n) * 0x20)

#define HPET_Tn_CMP(n)         (0x108 + (n) * 0x20)

#define HPET_DEFAULT_BASE       0xfed00000

/* Number of available HPET timers. */
extern unsigned int hpet_nr_timers;

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

/**
 * Fetch the HPET main counter register.
 */
static inline uint64_t hpet_read_counter(void)
{
    if ( IS_DEFINED(CONFIG_64BIT) )
        return hpet_read64(HPET_COUNTER);
    else
    {
        uint32_t lo, hi;

        do {
            hi = hpet_read32(HPET_COUNTER + 4);
            lo = hpet_read32(HPET_COUNTER);
        } while ( hi != hpet_read32(HPET_COUNTER + 4) );

        return ((uint64_t)hi << 32) | lo;
    }
}

/**
 * Setup and enable a specific HPET timer.
 */
void hpet_init_timer(unsigned int nr, unsigned int irq, uint64_t ticks,
                     bool level, bool periodic, bool mode32bit);

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
