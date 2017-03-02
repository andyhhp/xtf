/*
 * Xen x86 public cpufeatureset interface
 */

#ifndef XEN_PUBLIC_ARCH_X86_CPUFEATURESET_H
#define XEN_PUBLIC_ARCH_X86_CPUFEATURESET_H

/*
 * A featureset is a bitmap of x86 features, represented as a collection of
 * 32bit words.
 *
 * Words are as specified in vendors programming manuals, and shall not
 * contain any synthesied values.  New words may be added to the end of
 * featureset.
 *
 * All featureset words currently originate from leaves specified for the
 * CPUID instruction, but this is not preclude other sources of information.
 */

/* Intel-defined CPU features, CPUID level 0x00000001.edx, word 0 */
#define X86_FEATURE_FPU           (0*32+ 0) /* Onboard FPU */
#define X86_FEATURE_VME           (0*32+ 1) /* Virtual Mode Extensions */
#define X86_FEATURE_DE            (0*32+ 2) /* Debugging Extensions */
#define X86_FEATURE_PSE           (0*32+ 3) /* Page Size Extensions */
#define X86_FEATURE_TSC           (0*32+ 4) /* Time Stamp Counter */
#define X86_FEATURE_MSR           (0*32+ 5) /* Model-Specific Registers, RDMSR, WRMSR */
#define X86_FEATURE_PAE           (0*32+ 6) /* Physical Address Extensions */
#define X86_FEATURE_MCE           (0*32+ 7) /* Machine Check Architecture */
#define X86_FEATURE_CX8           (0*32+ 8) /* CMPXCHG8 instruction */
#define X86_FEATURE_APIC          (0*32+ 9) /* Onboard APIC */
#define X86_FEATURE_SEP           (0*32+11) /* SYSENTER/SYSEXIT */
#define X86_FEATURE_MTRR          (0*32+12) /* Memory Type Range Registers */
#define X86_FEATURE_PGE           (0*32+13) /* Page Global Enable */
#define X86_FEATURE_MCA           (0*32+14) /* Machine Check Architecture */
#define X86_FEATURE_CMOV          (0*32+15) /* CMOV instruction (FCMOVCC and FCOMI too if FPU present) */
#define X86_FEATURE_PAT           (0*32+16) /* Page Attribute Table */
#define X86_FEATURE_PSE36         (0*32+17) /* 36-bit PSEs */
#define X86_FEATURE_CLFLUSH       (0*32+19) /* CLFLUSH instruction */
#define X86_FEATURE_DS            (0*32+21) /* Debug Store */
#define X86_FEATURE_ACPI          (0*32+22) /* ACPI via MSR */
#define X86_FEATURE_MMX           (0*32+23) /* Multimedia Extensions */
#define X86_FEATURE_FXSR          (0*32+24) /* FXSAVE and FXRSTOR instructions */
#define X86_FEATURE_SSE           (0*32+25) /* Streaming SIMD Extensions */
#define X86_FEATURE_SSE2          (0*32+26) /* Streaming SIMD Extensions-2 */
#define X86_FEATURE_HTT           (0*32+28) /* Hyper-Threading Technology */
#define X86_FEATURE_TM1           (0*32+29) /* Thermal Monitor 1 */
#define X86_FEATURE_PBE           (0*32+31) /* Pending Break Enable */

/* Intel-defined CPU features, CPUID level 0x00000001.ecx, word 1 */
#define X86_FEATURE_SSE3          (1*32+ 0) /* Streaming SIMD Extensions-3 */
#define X86_FEATURE_PCLMULQDQ     (1*32+ 1) /* Carry-less mulitplication */
#define X86_FEATURE_DTES64        (1*32+ 2) /* 64-bit Debug Store */
#define X86_FEATURE_MONITOR       (1*32+ 3) /* Monitor/Mwait support */
#define X86_FEATURE_DSCPL         (1*32+ 4) /* CPL Qualified Debug Store */
#define X86_FEATURE_VMX           (1*32+ 5) /* Virtual Machine Extensions */
#define X86_FEATURE_SMX           (1*32+ 6) /* Safer Mode Extensions */
#define X86_FEATURE_EIST          (1*32+ 7) /* Enhanced SpeedStep */
#define X86_FEATURE_TM2           (1*32+ 8) /* Thermal Monitor 2 */
#define X86_FEATURE_SSSE3         (1*32+ 9) /* Supplemental Streaming SIMD Extensions-3 */
#define X86_FEATURE_FMA           (1*32+12) /* Fused Multiply Add */
#define X86_FEATURE_CX16          (1*32+13) /* CMPXCHG16B */
#define X86_FEATURE_XTPR          (1*32+14) /* Send Task Priority Messages */
#define X86_FEATURE_PDCM          (1*32+15) /* Perf/Debug Capability MSR */
#define X86_FEATURE_PCID          (1*32+17) /* Process Context ID */
#define X86_FEATURE_DCA           (1*32+18) /* Direct Cache Access */
#define X86_FEATURE_SSE4_1        (1*32+19) /* Streaming SIMD Extensions 4.1 */
#define X86_FEATURE_SSE4_2        (1*32+20) /* Streaming SIMD Extensions 4.2 */
#define X86_FEATURE_X2APIC        (1*32+21) /* Extended xAPIC */
#define X86_FEATURE_MOVBE         (1*32+22) /* movbe instruction */
#define X86_FEATURE_POPCNT        (1*32+23) /* POPCNT instruction */
#define X86_FEATURE_TSC_DEADLINE  (1*32+24) /* TSC Deadline Timer */
#define X86_FEATURE_AESNI         (1*32+25) /* AES instructions */
#define X86_FEATURE_XSAVE         (1*32+26) /* XSAVE/XRSTOR/XSETBV/XGETBV */
#define X86_FEATURE_OSXSAVE       (1*32+27) /* OSXSAVE */
#define X86_FEATURE_AVX           (1*32+28) /* Advanced Vector Extensions */
#define X86_FEATURE_F16C          (1*32+29) /* Half-precision convert instruction */
#define X86_FEATURE_RDRAND        (1*32+30) /* Digital Random Number Generator */
#define X86_FEATURE_HYPERVISOR    (1*32+31) /* Running under some hypervisor */

/* AMD-defined CPU features, CPUID level 0x80000001.edx, word 2 */
#define X86_FEATURE_SYSCALL       (2*32+11) /* SYSCALL/SYSRET */
#define X86_FEATURE_NX            (2*32+20) /* Execute Disable */
#define X86_FEATURE_MMXEXT        (2*32+22) /* AMD MMX extensions */
#define X86_FEATURE_FFXSR         (2*32+25) /* FFXSR instruction optimizations */
#define X86_FEATURE_PAGE1GB       (2*32+26) /* 1Gb large page support */
#define X86_FEATURE_RDTSCP        (2*32+27) /* RDTSCP */
#define X86_FEATURE_LM            (2*32+29) /* Long Mode (x86-64) */
#define X86_FEATURE_3DNOWEXT      (2*32+30) /* AMD 3DNow! extensions */
#define X86_FEATURE_3DNOW         (2*32+31) /* 3DNow! */

/* AMD-defined CPU features, CPUID level 0x80000001.ecx, word 3 */
#define X86_FEATURE_LAHF_LM       (3*32+ 0) /* LAHF/SAHF in long mode */
#define X86_FEATURE_CMP_LEGACY    (3*32+ 1) /* If yes HyperThreading not valid */
#define X86_FEATURE_SVM           (3*32+ 2) /* Secure virtual machine */
#define X86_FEATURE_EXTAPIC       (3*32+ 3) /* Extended APIC space */
#define X86_FEATURE_CR8_LEGACY    (3*32+ 4) /* CR8 in 32-bit mode */
#define X86_FEATURE_ABM           (3*32+ 5) /* Advanced bit manipulation */
#define X86_FEATURE_SSE4A         (3*32+ 6) /* SSE-4A */
#define X86_FEATURE_MISALIGNSSE   (3*32+ 7) /* Misaligned SSE mode */
#define X86_FEATURE_3DNOWPREFETCH (3*32+ 8) /* 3DNow prefetch instructions */
#define X86_FEATURE_OSVW          (3*32+ 9) /* OS Visible Workaround */
#define X86_FEATURE_IBS           (3*32+10) /* Instruction Based Sampling */
#define X86_FEATURE_XOP           (3*32+11) /* extended AVX instructions */
#define X86_FEATURE_SKINIT        (3*32+12) /* SKINIT/STGI instructions */
#define X86_FEATURE_WDT           (3*32+13) /* Watchdog timer */
#define X86_FEATURE_LWP           (3*32+15) /* Light Weight Profiling */
#define X86_FEATURE_FMA4          (3*32+16) /* 4 operands MAC instructions */
#define X86_FEATURE_NODEID_MSR    (3*32+19) /* NodeId MSR */
#define X86_FEATURE_TBM           (3*32+21) /* trailing bit manipulations */
#define X86_FEATURE_TOPOEXT       (3*32+22) /* topology extensions CPUID leafs */
#define X86_FEATURE_DBEXT         (3*32+26) /* data breakpoint extension */
#define X86_FEATURE_MONITORX      (3*32+29) /* MONITOR extension (MONITORX/MWAITX) */

/* Intel-defined CPU features, CPUID level 0x0000000D:1.eax, word 4 */
#define X86_FEATURE_XSAVEOPT      (4*32+ 0) /* XSAVEOPT instruction */
#define X86_FEATURE_XSAVEC        (4*32+ 1) /* XSAVEC/XRSTORC instructions */
#define X86_FEATURE_XGETBV1       (4*32+ 2) /* XGETBV with %ecx=1 */
#define X86_FEATURE_XSAVES        (4*32+ 3) /* XSAVES/XRSTORS instructions */

/* Intel-defined CPU features, CPUID level 0x00000007:0.ebx, word 5 */
#define X86_FEATURE_FSGSBASE      (5*32+ 0) /* {RD,WR}{FS,GS}BASE instructions */
#define X86_FEATURE_TSC_ADJUST    (5*32+ 1) /* TSC_ADJUST MSR available */
#define X86_FEATURE_SGX           (5*32+ 2) /* Software Guard extensions */
#define X86_FEATURE_BMI1          (5*32+ 3) /* 1st bit manipulation extensions */
#define X86_FEATURE_HLE           (5*32+ 4) /* Hardware Lock Elision */
#define X86_FEATURE_AVX2          (5*32+ 5) /* AVX2 instructions */
#define X86_FEATURE_FDP_EXCP_ONLY (5*32+ 6) /* x87 FDP only updated on exception. */
#define X86_FEATURE_SMEP          (5*32+ 7) /* Supervisor Mode Execution Protection */
#define X86_FEATURE_BMI2          (5*32+ 8) /* 2nd bit manipulation extensions */
#define X86_FEATURE_ERMS          (5*32+ 9) /* Enhanced REP MOVSB/STOSB */
#define X86_FEATURE_INVPCID       (5*32+10) /* Invalidate Process Context ID */
#define X86_FEATURE_RTM           (5*32+11) /* Restricted Transactional Memory */
#define X86_FEATURE_PQM           (5*32+12) /* Platform QoS Monitoring */
#define X86_FEATURE_NO_FPU_SEL    (5*32+13) /* FPU CS/DS stored as zero */
#define X86_FEATURE_MPX           (5*32+14) /* Memory Protection Extensions */
#define X86_FEATURE_PQE           (5*32+15) /* Platform QoS Enforcement */
#define X86_FEATURE_RDSEED        (5*32+18) /* RDSEED instruction */
#define X86_FEATURE_ADX           (5*32+19) /* ADCX, ADOX instructions */
#define X86_FEATURE_SMAP          (5*32+20) /* Supervisor Mode Access Prevention */
#define X86_FEATURE_PCOMMIT       (5*32+22) /* PCOMMIT instruction */
#define X86_FEATURE_CLFLUSHOPT    (5*32+23) /* CLFLUSHOPT instruction */
#define X86_FEATURE_CLWB          (5*32+24) /* CLWB instruction */
#define X86_FEATURE_SHA           (5*32+29) /* SHA1 & SHA256 instructions */

/* Intel-defined CPU features, CPUID level 0x00000007:0.ecx, word 6 */
#define X86_FEATURE_PREFETCHWT1   (6*32+ 0) /* PREFETCHWT1 instruction */
#define X86_FEATURE_UMIP          (6*32+ 2) /* User-Mode Instruction Prevention */
#define X86_FEATURE_PKU           (6*32+ 3) /* Protection Keys for Userspace */
#define X86_FEATURE_OSPKE         (6*32+ 4) /* OS Protection Keys Enable */

/* AMD-defined CPU features, CPUID level 0x80000007.edx, word 7 */
#define X86_FEATURE_ITSC          (7*32+ 8) /* Invariant TSC */
#define X86_FEATURE_EFRO          (7*32+10) /* APERF/MPERF Read Only interface */

/* AMD-defined CPU features, CPUID level 0x80000008.ebx, word 8 */
#define X86_FEATURE_CLZERO        (8*32+ 0) /* CLZERO instruction */

#endif /* XEN_PUBLIC_ARCH_X86_CPUFEATURESET_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
