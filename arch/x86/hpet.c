/**
 * @file arch/x86/hpet.c
 *
 * Basic x86 HPET driver.
 */

#include <xtf/lib.h>

#include <xen/errno.h>

#include <arch/hpet.h>

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

    return 0;
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
