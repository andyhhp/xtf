#ifndef XTF_X86_CPUID_H
#define XTF_X86_CPUID_H

typedef void (*cpuid_fn_t)(uint32_t leaf,
                           uint32_t *eax, uint32_t *ebx,
                           uint32_t *ecx, uint32_t *edx);
typedef void (*cpuid_count_fn_t)(uint32_t leaf, uint32_t subleaf,
                                 uint32_t *eax, uint32_t *ebx,
                                 uint32_t *ecx, uint32_t *edx);

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
