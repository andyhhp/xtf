/**
 * @file tests/rtm-check/main.c
 * @ref test-rtm-check
 *
 * @page test-rtm-check rtm-check
 *
 * Probe the RTM behaviour on the system, independently of CPUID settings.
 * Classifies as usable, unavailable (@#UD) or always aborting.
 *
 * @see tests/rtm-check/main.c
 */
#include <xtf.h>

const char test_title[] = "Test RTM behaviour";

void test_main(void)
{
    printk("CPUID: HLE %u, RTM %u\n",
           cpu_has_hle, cpu_has_rtm);

    for ( int i = 0; i < 1000; ++i )
    {
        unsigned int xstatus = _xbegin_safe();

        if ( xstatus == _XBEGIN_STARTED )
        {
            _xend();
            return xtf_success("RTM usable\n");
        }
        else if ( xstatus == _XBEGIN_UD )
            return xtf_success("RTM unavailable\n");
    }

    xtf_success("RTM always aborting\n");
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
