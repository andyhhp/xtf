/**
 * @file arch/x86/include/arch/msr.h
 *
 * Model Specific Register mnemonics and bit definitions.
 */
#ifndef XTF_X86_MSR_INDEX_H
#define XTF_X86_MSR_INDEX_H

#include <xtf/numbers.h>

#define MSR_APICBASE                    0x0000001b
#define MSR_APICBASE_BSP                (_AC(1, L) << 8)
#define MSR_APICBASE_EXTD               (_AC(1, L) << 10)
#define MSR_APICBASE_ENABLE             (_AC(1, L) << 11)

#define MSR_FEATURE_CONTROL             0x0000003a

#define MSR_PMC(n)                     (0x000000c1 + (n))

#define MSR_INTEL_PLATFORM_INFO         0x000000ce
#define _MSR_PLATFORM_INFO_CPUID_FAULTING       31
#define MSR_PLATFORM_INFO_CPUID_FAULTING        (1ULL << _MSR_PLATFORM_INFO_CPUID_FAULTING)

#define MSR_INTEL_MISC_FEATURES_ENABLES 0x00000140
#define _MSR_MISC_FEATURES_CPUID_FAULTING        0
#define MSR_MISC_FEATURES_CPUID_FAULTING         (1ULL << _MSR_MISC_FEATURES_CPUID_FAULTING)

#define MSR_PERFEVTSEL(n)              (0x00000186 + (n))

#define MSR_DEBUGCTL                    0x000001d9
#define _MSR_DEBUGCTL_LBR               0 /* Last Branch Record. */
#define MSR_DEBUGCTL_LBR                (_AC(1, L) << _MSR_DEBUGCTL_LBR)

#define MSR_FIXED_CTR(n)               (0x00000309 + (n))
#define MSR_PERF_CAPABILITIES           0x00000345
#define MSR_FIXED_CTR_CTRL              0x0000038d
#define MSR_PERF_GLOBAL_STATUS          0x0000038e
#define MSR_PERF_GLOBAL_CTRL            0x0000038f
#define MSR_PERF_GLOBAL_OVF_CTRL        0x00000390

#define MSR_VMX_BASIC                   0x00000480

#define MSR_A_PMC(n)                   (0x000004c1 + (n))

#define MSR_X2APIC_REGS                 0x00000800

#define MSR_EFER                        0xc0000080 /* Extended Feature register. */
#define _EFER_SCE                       0  /* SYSCALL Enable. */
#define EFER_SCE                        (_AC(1, L) << _EFER_SCE)
#define _EFER_LME                       8  /* Long mode enable. */
#define EFER_LME                        (_AC(1, L) << _EFER_LME)
#define _EFER_LMA                       10 /* Long mode Active. */
#define EFER_LMA                        (_AC(1, L) << _EFER_LMA)
#define _EFER_NXE                       11 /* No-Execute Enable. */
#define EFER_NXE                        (_AC(1, L) << _EFER_NXE)
#define _EFER_SVME                      12 /* Secure Virtual Machine Enable. */
#define EFER_SVME                       (_AC(1, L) << _EFER_SVME)
#define _EFER_LMSLE                     13 /* Long Mode Segment Limit Enable. */
#define EFER_LMSLE                      (_AC(1, L) << _EFER_LMSLE)
#define _EFER_FFXSR                     14 /* Fast FXSAVE/FXRSTOR. */
#define EFER_FFXSR                      (_AC(1, L) << _EFER_FFXSR)
#define _EFER_TCE                       15 /* Translation Cache Extension. */
#define EFER_TCE                        (_AC(1, L) << _EFER_TCE)

#define MSR_STAR                        0xc0000081
#define MSR_LSTAR                       0xc0000082
#define MSR_CSTAR                       0xc0000083
#define MSR_FMASK                       0xc0000084

#define MSR_FS_BASE                     0xc0000100
#define MSR_GS_BASE                     0xc0000101
#define MSR_SHADOW_GS_BASE              0xc0000102

#endif /* XTF_X86_MSR_INDEX_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */

