/**
 * @file tests/msr/main.c
 * @ref test-msr
 *
 * @page test-msr MSR
 *
 * Prints the values of all MSRs which are readable to the guest.
 *
 * @warning As this probes all MSR indicies, it can cause substantial logspam
 * in Xen from extable fixup, depending on log level.
 *
 * @see tests/msr/main.c
 */
#include <xtf.h>

const char test_title[] = "Guest MSR information";

void test_main(void)
{
    unsigned int idx = 0;
    uint64_t val;

    do {
        if ( !rdmsr_safe(idx, &val) )
            printk("  %08x -> %016"PRIx64"\n", idx, val);

        idx++;

        switch ( idx )
        {
        case 0x2000:
            idx = 0x40000000;
            break;

        case 0x40000100:
            idx = 0xC0000000;
            break;
        }

    } while ( idx <= 0xC0001fff );

    xtf_success(NULL);
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
