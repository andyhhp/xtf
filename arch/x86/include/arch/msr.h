/**
 * @file arch/x86/include/arch/msr.h
 *
 * Misc C-level infrastructure for MSRs.
 */
#ifndef XTF_X86_MSR_H
#define XTF_X86_MSR_H

#include <xtf/extable.h>
#include <xtf/types.h>

#include <xen/arch-x86/xen.h>

#include <arch/msr-index.h>

/**
 * Thin wrapper around an `rdmsr` instruction.  May crash with @#GP[0].
 */
static inline uint64_t rdmsr(uint32_t idx)
{
    uint32_t lo, hi;

    asm volatile ("rdmsr": "=a" (lo), "=d" (hi): "c" (idx));

    return (((uint64_t)hi) << 32) | lo;
}

/**
 * Wrapper around `rdmsr` which safely catches @#GP[0].
 *
 * @param idx MSR to read
 * @param [out] val Value, if no fault occurred.
 * @return boolean indicating whether the read faulted.
 */
static inline bool rdmsr_safe(uint32_t idx, uint64_t *val)
{
    uint32_t lo, hi, new_idx;

    asm volatile ("1: rdmsr; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[hnd])
                  : "=a" (lo), "=d" (hi), "=c" (new_idx)
                  : "c" (idx), [hnd] "p" (ex_rdmsr_safe));

    bool fault = idx != new_idx;

    if ( !fault )
        *val = (((uint64_t)hi) << 32) | lo;

    return fault;
}

/**
 * Thin wrapper around an `wrmsr` instruction.  May crash with @#GP[0].
 */
static inline void wrmsr(uint32_t idx, uint64_t val)
{
    asm volatile ("wrmsr":
                  : "c" (idx), "a" ((uint32_t)val),
                    "d" ((uint32_t)(val >> 32)));
}

/**
 * Wrapper around `wrmsr` which safely catches @#GP[0].
 *
 * @param idx MSR to write
 * @param val Value to write
 * @return boolean indicating whether the write faulted.
 */
static inline bool wrmsr_safe(uint32_t idx, uint64_t val)
{
    uint32_t new_idx;

    asm volatile ("1: wrmsr; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[hnd])
                  : "=c" (new_idx)
                  : "c" (idx), "a" ((uint32_t)val),
                    "d" ((uint32_t)(val >> 32)),
                    [hnd] "p" (ex_wrmsr_safe));

    return idx != new_idx;
}

/*
 * Types wrapping MSR content.
 */
typedef union msr_feature_control {
    uint64_t raw;
    struct {
        bool lock:1,
            vmxon_inside_smx:1,
            vmxon_outside_smx:1;
    };
} msr_feature_control_t;

typedef union msr_vmx_basic {
    uint64_t raw;
    struct {
        uint32_t      vmcs_rev_id:31;
        bool                  mbz:1;
        uint32_t        vmcs_size:13;
        uint32_t                 :3;
        bool          paddr_32bit:1;
        bool             smm_dual:1;
        uint32_t    vmcs_mem_type:4;
        bool     inouts_exit_info:1;
        bool            true_ctls:1;
    };
} msr_vmx_basic_t;

/*
 * Library logic for MSRs.
 */
struct xtf_msr_consistency_test_data
{
    uint32_t msr;

    const struct xtf_msr_consistency_test_vals
    {
        uint64_t val;
        bool pred;
    } *vals;

    size_t nr_vals;
};

/**
 * Run consistency tests as described by @p t
 *
 * Some MSRs may be passed directly through to guests for performance reasons.
 * This introduces an extra level of complexity for context switching an
 * emulation purposes.
 *
 * To check that synchronisation is working properly, @p t describes an MSR
 * and an array of predicated values.  For each value where the predicate is
 * true, mix regular and forced reads and writes to check that values written
 * via one mechanism become visible via the other.
 *
 * This logic is only applicable to read/write MSRs which expect to retain
 * their written values on subsequent reads, and for values which will succeed
 * when written.
 */
void xtf_msr_consistency_test(const struct xtf_msr_consistency_test_data *t);

#endif /* XTF_X86_MSR_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
