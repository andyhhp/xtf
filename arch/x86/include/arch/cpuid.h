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

enum x86_vendor
{
    X86_VENDOR_UNKNOWN,
    X86_VENDOR_INTEL,
    X86_VENDOR_AMD,
};

extern enum x86_vendor x86_vendor;
extern unsigned int max_leaf, max_extd_leaf;
extern unsigned int x86_family, x86_model, x86_stepping;
extern unsigned int maxphysaddr, maxvirtaddr;

static inline bool vendor_is(enum x86_vendor v)
{
    return x86_vendor == v;
}

#define vendor_is_intel         vendor_is(X86_VENDOR_INTEL)
#define vendor_is_amd           vendor_is(X86_VENDOR_AMD)


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

#define cpu_has_fpu             cpu_has(X86_FEATURE_FPU)
#define cpu_has_vme             cpu_has(X86_FEATURE_VME)
#define cpu_has_de              cpu_has(X86_FEATURE_DE)
#define cpu_has_pse             cpu_has(X86_FEATURE_PSE)
#define cpu_has_tsc             cpu_has(X86_FEATURE_TSC)
#define cpu_has_pae             cpu_has(X86_FEATURE_PAE)
#define cpu_has_mce             cpu_has(X86_FEATURE_MCE)
#define cpu_has_pge             cpu_has(X86_FEATURE_PGE)
#define cpu_has_mca             cpu_has(X86_FEATURE_MCA)
#define cpu_has_pat             cpu_has(X86_FEATURE_PAT)
#define cpu_has_pse36           cpu_has(X86_FEATURE_PSE36)
#define cpu_has_ds              cpu_has(X86_FEATURE_DS)
#define cpu_has_mmx             cpu_has(X86_FEATURE_MMX)
#define cpu_has_fxsr            cpu_has(X86_FEATURE_FXSR)

#define cpu_has_sse             cpu_has(X86_FEATURE_SSE)
#define cpu_has_sse2            cpu_has(X86_FEATURE_SSE2)
#define cpu_has_vmx             cpu_has(X86_FEATURE_VMX)
#define cpu_has_smx             cpu_has(X86_FEATURE_SMX)
#define cpu_has_pcid            cpu_has(X86_FEATURE_PCID)
#define cpu_has_x2apic          cpu_has(X86_FEATURE_X2APIC)
#define cpu_has_xsave           cpu_has(X86_FEATURE_XSAVE)
#define cpu_has_avx             cpu_has(X86_FEATURE_AVX)

#define cpu_has_syscall         cpu_has(X86_FEATURE_SYSCALL)
#define cpu_has_nx              cpu_has(X86_FEATURE_NX)
#define cpu_has_page1gb         cpu_has(X86_FEATURE_PAGE1GB)
#define cpu_has_lm              cpu_has(X86_FEATURE_LM)

#define cpu_has_svm             cpu_has(X86_FEATURE_SVM)

#define cpu_has_fsgsbase        cpu_has(X86_FEATURE_FSGSBASE)
#define cpu_has_hle             cpu_has(X86_FEATURE_HLE)
#define cpu_has_smep            cpu_has(X86_FEATURE_SMEP)
#define cpu_has_rtm             cpu_has(X86_FEATURE_RTM)
#define cpu_has_smap            cpu_has(X86_FEATURE_SMAP)

#define cpu_has_umip            cpu_has(X86_FEATURE_UMIP)
#define cpu_has_pku             cpu_has(X86_FEATURE_PKU)

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
