/**
 * @file arch/x86/io-apic.c
 *
 * Basic x86 IO-APIC driver.
 */

#include <xtf/lib.h>

#include <xen/errno.h>

#include <arch/io-apic.h>

static unsigned int nr_entries;

int ioapic_init(void)
{
    uint32_t id = ioapic_read32(IOAPIC_ID);

    /* Bare MMIO? */
    if ( id == ~0u )
        return -ENODEV;

    nr_entries = MASK_EXTR(ioapic_read32(IOAPIC_VERSION),
                           IOAPIC_MAXREDIR_MASK) + 1;

    return 0;
}

int ioapic_set_mask(unsigned int entry, bool mask)
{
    uint32_t redir;

    if ( entry >= nr_entries )
        return -EINVAL;

    redir = ioapic_read32(IOAPIC_REDIR_ENTRY(entry));
    redir &= ~(1u << IOAPIC_REDIR_MASK_SHIFT);
    if ( mask )
        redir |= 1u << IOAPIC_REDIR_MASK_SHIFT;
    ioapic_write32(IOAPIC_REDIR_ENTRY(entry), redir);

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
