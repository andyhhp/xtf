/**
 * @file arch/x86/apic.c
 *
 * Basic x86 APIC driver.
 */
#include <xtf/lib.h>

#include <arch/apic.h>
#include <arch/cpuid.h>
#include <arch/lib.h>
#include <arch/page.h>

enum apic_mode cur_apic_mode;

static enum apic_mode apicbase_to_mode(uint64_t apicbase)
{
    switch ( apicbase & (MSR_APICBASE_EXTD | MSR_APICBASE_ENABLE) )
    {
    case 0:
        return APIC_MODE_DISABLED;

    case MSR_APICBASE_ENABLE:
        return APIC_MODE_XAPIC;

    case MSR_APICBASE_EXTD | MSR_APICBASE_ENABLE:
        return APIC_MODE_X2APIC;

    default:
        return APIC_MODE_NONE;
    }
}

int apic_init(enum apic_mode mode)
{
    uint64_t msrval;

    ASSERT(mode > APIC_MODE_NONE);

    /*
     * First pass through this function.  Try to calculate what is available
     * (if anything).
     */
    if ( cur_apic_mode == APIC_MODE_UNKNOWN )
    {
        if ( rdmsr_safe(MSR_APICBASE, &msrval) )
            cur_apic_mode = APIC_MODE_NONE;
        else
        {
            cur_apic_mode = apicbase_to_mode(msrval);

            /* Set the MMIO base back to default if necessary. */
            if ( (msrval & PAGE_MASK) != APIC_DEFAULT_BASE )
            {
                msrval &= ~PAGE_MASK;
                msrval |= APIC_DEFAULT_BASE;

                wrmsr(MSR_APICBASE, msrval);
            }
        }
    }

    /*
     * Bail if there is no APIC, or X2APIC was requested and hardware doesn't
     * support it.
     */
    if ( (cur_apic_mode == APIC_MODE_NONE) ||
         (mode == APIC_MODE_X2APIC && !cpu_has_x2apic) )
        return -ENODEV;

    /*
     * Attempt to switch to the requested mode.  For simplicity, this always
     * starts by disabling, then cycling through XAPIC and X2APIC if
     * applicable.
     */
    if ( mode != cur_apic_mode )
    {
        msrval = rdmsr(MSR_APICBASE) &
            ~(MSR_APICBASE_EXTD | MSR_APICBASE_ENABLE);

        wrmsr(MSR_APICBASE, msrval);

        if ( mode == APIC_MODE_XAPIC || mode == APIC_MODE_X2APIC )
            wrmsr(MSR_APICBASE, msrval | MSR_APICBASE_ENABLE);

        if ( mode == APIC_MODE_X2APIC )
            wrmsr(MSR_APICBASE, msrval | MSR_APICBASE_ENABLE | MSR_APICBASE_EXTD);

        cur_apic_mode = mode;
    }

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
