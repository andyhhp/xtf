/**
 * @file arch/x86/hpet.c
 *
 * Basic x86 HPET driver.
 */

#include <xtf/lib.h>

#include <xen/errno.h>

#include <arch/hpet.h>

unsigned int hpet_nr_timers;

union hpet_timer {
    uint32_t raw;
    struct {
        uint8_t                 :1;
        bool level              :1;
        bool enabled            :1;
        bool periodic           :1;
        uint8_t                 :4;
        bool mode_32bit         :1;
        uint8_t irq             :5;
        uint32_t                :18;
    };
};

int hpet_init(void)
{
    uint64_t id = hpet_read64(HPET_ID);

    /* Bare MMIO? */
    if ( id == ~0ull )
        return -ENODEV;

    uint32_t period = id >> 32;

    /* Sanity check main counter tick period. */
    if ( period == 0 || period > HPET_ID_MAX_PERIOD )
        return -ENODEV;

    /* Get number of timers. */
    hpet_nr_timers = MASK_EXTR(id, HPET_ID_NUMBER_MASK) + 1;

    return 0;
}

void hpet_init_timer(unsigned int nr, unsigned int irq, uint64_t ticks,
                     bool level, bool periodic, bool mode32bit)
{
    uint64_t cfg;
    union hpet_timer tm = {};

    tm.level = level;
    tm.enabled = true;
    tm.periodic = periodic;
    tm.mode_32bit = mode32bit;
    tm.irq = irq;

    /*
     * Disable interrupts and reset main counter.
     *
     * Note that this is a testing HPET implementation and ATM we only expect
     * a single timer to be tested simultaneously.
     */
    cfg = hpet_read64(HPET_CFG);
    hpet_write64(HPET_CFG, cfg & ~HPET_CFG_ENABLE);
    hpet_write64(HPET_COUNTER, 0);

    /* Configure timer and setup comparator. */
    hpet_write32(HPET_Tn_CFG(nr), tm.raw);
    hpet_write64(HPET_Tn_CMP(nr), ticks);

    /* Enable main counter. */
    hpet_write64(HPET_CFG, cfg | HPET_CFG_ENABLE);
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
