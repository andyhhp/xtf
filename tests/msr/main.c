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
 * @sa tests/msr/main.c
 */
#include <xtf.h>

static int rdmsr_safe(uint32_t msr, uint64_t *val)
{
    uint32_t lo, hi;
    int ret;

    asm volatile ("1: rdmsr;"
                  "jmp 3f; 2:"
                  "xor %[lo], %[lo];"
                  "xor %[hi], %[hi];"
                  "mov %[err], %[res];"
                  "3:"
                  _ASM_EXTABLE(1b, 2b)
                  : [lo] "=a" (lo), [hi] "=d" (hi), [res] "=&q" (ret)
                  : "c" (msr), [err] "ir" (-EFAULT), "2" (0));

    *val = ((uint64_t)hi) << 32 | lo;
    return ret;
}

void test_main(void)
{
    unsigned int idx = 0;
    uint64_t val;

    printk("Guest MSR information\n");

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
