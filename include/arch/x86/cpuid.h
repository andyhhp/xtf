#ifndef XTF_X86_CPUID_H
#define XTF_X86_CPUID_H

#include <xtf/types.h>
#include <xtf/numbers.h>

#include <xen/arch-x86/cpufeatureset.h>

typedef void (*cpuid_fn_t)(uint32_t leaf,
                           uint32_t *eax, uint32_t *ebx,
                           uint32_t *ecx, uint32_t *edx);
typedef void (*cpuid_count_fn_t)(uint32_t leaf, uint32_t subleaf,
                                 uint32_t *eax, uint32_t *ebx,
                                 uint32_t *ecx, uint32_t *edx);

#define cpufeat_word(idx)       ((idx) / 32)
#define cpufeat_bit(idx)        ((idx) % 32)
#define cpufeat_mask(idx)       (_AC(1, U) << cpufeat_bit(idx))

#define FEATURESET_1d           cpufeat_word(X86_FEATURE_FPU)
#define FEATURESET_1c           cpufeat_word(X86_FEATURE_SSE3)
#define FEATURESET_e1d          cpufeat_word(X86_FEATURE_SYSCALL)
#define FEATURESET_e1c          cpufeat_word(X86_FEATURE_LAHF_LM)
#define FEATURESET_Da1          cpufeat_word(X86_FEATURE_XSAVEOPT)
#define FEATURESET_7b0          cpufeat_word(X86_FEATURE_FSGSBASE)
#define FEATURESET_7c0          cpufeat_word(X86_FEATURE_PREFETCHWT1)
#define FEATURESET_e7d          cpufeat_word(X86_FEATURE_ITSC)
#define FEATURESET_e8b          cpufeat_word(X86_FEATURE_CLZERO)

#define FSCAPINTS               (FEATURESET_e8b + 1)

extern uint32_t x86_features[FSCAPINTS];

static inline bool cpu_has(unsigned int feature)
{
    return x86_features[cpufeat_word(feature)] & cpufeat_mask(feature);
}

#endif /* XTF_X86_CPUID_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
