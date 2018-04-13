/**
 * @file arch/x86/io-apic.c
 *
 * Basic x86 IO-APIC driver.
 */

#include <xtf/lib.h>

#include <xen/errno.h>

#include <arch/io-apic.h>

int ioapic_init(void)
{
    uint32_t id = ioapic_read32(IOAPIC_ID);

    /* Bare MMIO? */
    if ( id == ~0u )
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
