/**
 * @file tests/cpuid/main.c
 * @ref test-cpuid
 *
 * @page test-cpuid CPUID
 *
 * Prints all CPUID information visible to the guest.  PV guests dump both
 * native and emulated CPUID.
 *
 * @sa tests/cpuid/main.c
 */
#include <xtf.h>

static void dump_leaves(cpuid_count_fn_t cpuid_fn)
{
    uint32_t leaf = 0, subleaf = ~0U;

    uint64_t valid_xstate_leaves = 0;
    uint32_t max_leaf = 0, max_l7_subleaf = 0,
        max_hv_leaf = 0, max_hv2_leaf = 0, max_extd_leaf = 0;

    for ( ;; )
    {
        uint32_t eax, ebx, ecx, edx;

        cpuid_fn(leaf, subleaf, &eax, &ebx, &ecx, &edx);

        printk("  %08x:%08x -> %08x:%08x:%08x:%08x\n",
               leaf, subleaf, eax, ebx, ecx, edx);

        switch ( leaf )
        {
        case 0:
            max_leaf = eax;
            break;

        case 0x4:
            subleaf++;
            if ( (eax & 0x1f) != 0 )
                continue;
            break;

        case 0x7:
            if ( subleaf == 0 )
                max_l7_subleaf = eax;
            subleaf++;
            if ( subleaf <= max_l7_subleaf )
                continue;
            break;

        case 0xd:
            if ( subleaf == 0 )
                valid_xstate_leaves = ((uint64_t)edx) << 32 | eax;
            do
            {
                subleaf++;
            } while ( (subleaf < 63) &&
                      !(valid_xstate_leaves & (1ULL << subleaf)) );
            if ( subleaf < 63 )
                continue;
            break;

        case 0x40000000U:
            max_hv_leaf = eax;
            break;

        case 0x40000100U:
            max_hv2_leaf = eax;
            break;

        case 0x80000000U:
            max_extd_leaf = eax;
            break;
        }

        leaf++;
        if ( (leaf > 0) && (leaf < 0x40000000U) && (leaf > max_leaf) )
            leaf = 0x40000000U;

        if ( (leaf > 0x40000000U) && (leaf < 0x40000100U) && (leaf > max_hv_leaf) )
            leaf = 0x40000100U;

        if ( (leaf > 0x40000100U) && (leaf < 0x80000000U) && (leaf > max_hv2_leaf) )
            leaf = 0x80000000U;

        if ( (leaf > 0x80000000U) && (leaf > max_extd_leaf) )
            break;

        subleaf = ~0;
        if ( leaf == 4 || leaf == 7 || leaf == 0xd )
            subleaf = 0;
    }
}

void test_main(void)
{
    printk("Guest cpuid information\n");

    printk("Native cpuid:\n");
    dump_leaves(cpuid_count);

    if ( IS_DEFINED(CONFIG_PV) )
    {
        printk("Emulated cpuid:\n");
        dump_leaves(pv_cpuid_count);
    }

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
