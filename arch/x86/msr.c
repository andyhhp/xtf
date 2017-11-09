/**
 * @file arch/x86/msr.c
 *
 * Library logic for MSRs.
 */
#include <xtf/report.h>
#include <xtf/test.h>

#include <arch/msr.h>

static uint64_t force_rdmsr(uint32_t idx)
{
    uint32_t lo, hi;

    asm volatile (_ASM_XEN_FEP "rdmsr"
                  : "=a" (lo), "=d" (hi)
                  : "c" (idx));

    return (((uint64_t)hi) << 32) | lo;
}

static void force_wrmsr(uint32_t idx, uint64_t val)
{
    asm volatile (_ASM_XEN_FEP "wrmsr":
                  : "c" (idx), "a" ((uint32_t)val),
                    "d" ((uint32_t)(val >> 32)));
}

void xtf_msr_consistency_test(const struct xtf_msr_consistency_test_data *t)
{
    size_t i;
    uint64_t val;

    for ( i = 0; i < t->nr_vals; ++i )
    {
        if ( !t->vals[i].pred )
            continue;

        wrmsr(t->msr, t->vals[i].val);
        val = rdmsr(t->msr);

        if ( val != t->vals[i].val )
            xtf_failure("Fail: MSR %08x, real write, real read\n"
                        "  Got %016"PRIx64", Expected %016"PRIx64"\n",
                        t->msr, val, t->vals[i].val);

        if ( !xtf_has_fep )
            continue;

        val = force_rdmsr(t->msr);

        if ( val != t->vals[i].val )
            xtf_failure("Fail: MSR %08x, real write, emul read\n"
                        "  Got %016"PRIx64", Expected %016"PRIx64"\n",
                        t->msr, val, t->vals[i].val);
    }

    if ( !xtf_has_fep )
        return;

    for ( i = 0; i < t->nr_vals; ++i )
    {
        if ( !t->vals[i].pred )
            continue;

        force_wrmsr(t->msr, t->vals[i].val);
        val = rdmsr(t->msr);

        if ( val != t->vals[i].val )
            xtf_failure("Fail: MSR %08x, emul write, real read\n"
                        "  Got %016"PRIx64", Expected %016"PRIx64"\n",
                        t->msr, val, t->vals[i].val);

        val = force_rdmsr(t->msr);

        if ( val != t->vals[i].val )
            xtf_failure("Fail: MSR %08x, emul write, emul read\n"
                        "  Got %016"PRIx64", Expected %016"PRIx64"\n",
                        t->msr, val, t->vals[i].val);
    }
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
