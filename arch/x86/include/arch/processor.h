#ifndef XTF_X86_PROCESSOR_H
#define XTF_X86_PROCESSOR_H

/*
 * EFLAGS bits.
 */
#define X86_EFLAGS_CF           0x00000001 /* Carry Flag                */
#define X86_EFLAGS_MBS          0x00000002 /* Resvd bit                 */
#define X86_EFLAGS_PF           0x00000004 /* Parity Flag               */
#define X86_EFLAGS_AF           0x00000010 /* Auxillary carry Flag      */
#define X86_EFLAGS_ZF           0x00000040 /* Zero Flag                 */
#define X86_EFLAGS_SF           0x00000080 /* Sign Flag                 */
#define X86_EFLAGS_TF           0x00000100 /* Trap Flag                 */
#define X86_EFLAGS_IF           0x00000200 /* Interrupt Flag            */
#define X86_EFLAGS_DF           0x00000400 /* Direction Flag            */
#define X86_EFLAGS_OF           0x00000800 /* Overflow Flag             */
#define X86_EFLAGS_IOPL         0x00003000 /* IOPL mask                 */
#define X86_EFLAGS_NT           0x00004000 /* Nested Task               */
#define X86_EFLAGS_RF           0x00010000 /* Resume Flag               */
#define X86_EFLAGS_VM           0x00020000 /* Virtual Mode              */
#define X86_EFLAGS_AC           0x00040000 /* Alignment Check           */
#define X86_EFLAGS_VIF          0x00080000 /* Virtual Interrupt Flag    */
#define X86_EFLAGS_VIP          0x00100000 /* Virtual Interrupt Pending */
#define X86_EFLAGS_ID           0x00200000 /* CPUID detection flag      */

/*
 * CPU flags in CR0.
 */
#define X86_CR0_PE              0x00000001 /* Enable Protected Mode    (RW) */
#define X86_CR0_MP              0x00000002 /* Monitor Coprocessor      (RW) */
#define X86_CR0_EM              0x00000004 /* Require FPU Emulation    (RO) */
#define X86_CR0_TS              0x00000008 /* Task Switched            (RW) */
#define X86_CR0_ET              0x00000010 /* Extension type           (RO) */
#define X86_CR0_NE              0x00000020 /* Numeric Error Reporting  (RW) */
#define X86_CR0_WP              0x00010000 /* Supervisor Write Protect (RW) */
#define X86_CR0_AM              0x00040000 /* Alignment Checking       (RW) */
#define X86_CR0_NW              0x20000000 /* Not Write-Through        (RW) */
#define X86_CR0_CD              0x40000000 /* Cache Disable            (RW) */
#define X86_CR0_PG              0x80000000 /* Paging                   (RW) */

/*
 * CPU features in CR4.
 */
#define X86_CR4_VME             0x00000001  /* VM86 extensions                */
#define X86_CR4_PVI             0x00000002  /* Virtual interrupts flag        */
#define X86_CR4_TSD             0x00000004  /* Disable time stamp at ipl 3    */
#define X86_CR4_DE              0x00000008  /* Debugging extensions           */
#define X86_CR4_PSE             0x00000010  /* Page size extensions           */
#define X86_CR4_PAE             0x00000020  /* Physical address extensions    */
#define X86_CR4_MCE             0x00000040  /* Machine check                  */
#define X86_CR4_PGE             0x00000080  /* Global pages                   */
#define X86_CR4_PCE             0x00000100  /* Performance counters at ipl 3  */
#define X86_CR4_OSFXSR          0x00000200  /* Fast FPU save and restore      */
#define X86_CR4_OSXMMEXCPT      0x00000400  /* Unmasked SSE exceptions        */
#define X86_CR4_UMIP            0x00000800  /* UMIP                           */
#define X86_CR4_VMXE            0x00002000  /* VMX                            */
#define X86_CR4_SMXE            0x00004000  /* SMX                            */
#define X86_CR4_FSGSBASE        0x00010000  /* {rd,wr}{fs,gs}base             */
#define X86_CR4_PCIDE           0x00020000  /* PCID                           */
#define X86_CR4_OSXSAVE         0x00040000  /* XSAVE/XRSTOR                   */
#define X86_CR4_SMEP            0x00100000  /* SMEP                           */
#define X86_CR4_SMAP            0x00200000  /* SMAP                           */
#define X86_CR4_PKE             0x00400000  /* PKE                            */

/*
 * CPU features in XCR0.
 */
#define _XSTATE_FP                0
#define XSTATE_FP                 (1ULL << _XSTATE_FP)
#define _XSTATE_SSE               1
#define XSTATE_SSE                (1ULL << _XSTATE_SSE)
#define _XSTATE_YMM               2
#define XSTATE_YMM                (1ULL << _XSTATE_YMM)
#define _XSTATE_BNDREGS           3
#define XSTATE_BNDREGS            (1ULL << _XSTATE_BNDREGS)
#define _XSTATE_BNDCSR            4
#define XSTATE_BNDCSR             (1ULL << _XSTATE_BNDCSR)
#define _XSTATE_OPMASK            5
#define XSTATE_OPMASK             (1ULL << _XSTATE_OPMASK)
#define _XSTATE_ZMM               6
#define XSTATE_ZMM                (1ULL << _XSTATE_ZMM)
#define _XSTATE_HI_ZMM            7
#define XSTATE_HI_ZMM             (1ULL << _XSTATE_HI_ZMM)
#define _XSTATE_PKRU              9
#define XSTATE_PKRU               (1ULL << _XSTATE_PKRU)
#define _XSTATE_LWP               62
#define XSTATE_LWP                (1ULL << _XSTATE_LWP)

/*
 * Exception mnemonics.
 */
#define X86_EXC_DE             0 /* Divide Error. */
#define X86_EXC_DB             1 /* Debug Exception. */
#define X86_EXC_NMI            2 /* NMI. */
#define X86_EXC_BP             3 /* Breakpoint. */
#define X86_EXC_OF             4 /* Overflow. */
#define X86_EXC_BR             5 /* BOUND Range. */
#define X86_EXC_UD             6 /* Invalid Opcode. */
#define X86_EXC_NM             7 /* Device Not Available. */
#define X86_EXC_DF             8 /* Double Fault. */
#define X86_EXC_CSO            9 /* Coprocessor Segment Overrun. */
#define X86_EXC_TS            10 /* Invalid TSS. */
#define X86_EXC_NP            11 /* Segment Not Present. */
#define X86_EXC_SS            12 /* Stack-Segment Fault. */
#define X86_EXC_GP            13 /* General Porection Fault. */
#define X86_EXC_PF            14 /* Page Fault. */
#define X86_EXC_SPV           15 /* PIC Spurious Interrupt Vector. */
#define X86_EXC_MF            16 /* Maths fault (x87 FPU). */
#define X86_EXC_AC            17 /* Alignment Check. */
#define X86_EXC_MC            18 /* Machine Check. */
#define X86_EXC_XM            19 /* SIMD Exception. */
#define X86_EXC_VE            20 /* Virtualisation Exception. */

/* Bitmap of exceptions which have error codes. */
#define X86_EXC_HAVE_EC ((1 << X86_EXC_DF) | (1 << X86_EXC_TS) |    \
                         (1 << X86_EXC_NP) | (1 << X86_EXC_SS) |    \
                         (1 << X86_EXC_GP) | (1 << X86_EXC_PF) |    \
                         (1 << X86_EXC_AC))

/* Bitmap of exceptions which are classified as faults. */
#define X86_EXC_FAULTS ((1 << X86_EXC_DE)  | (1 << X86_EXC_BR) |    \
                        (1 << X86_EXC_UD)  | (1 << X86_EXC_NM) |    \
                        (1 << X86_EXC_CSO) | (1 << X86_EXC_TS) |    \
                        (1 << X86_EXC_NP)  | (1 << X86_EXC_SS) |    \
                        (1 << X86_EXC_GP)  | (1 << X86_EXC_PF) |    \
                        (1 << X86_EXC_MF)  | (1 << X86_EXC_AC) |    \
                        (1 << X86_EXC_XM)  | (1 << X86_EXC_VE))

/* Bitmap of exceptions which are classified as interrupts. */
#define X86_EXC_INTERRUPTS (1 << X86_EXC_NMI)

/* Bitmap of exceptions which are classified as traps. */
#define X86_EXC_TRAPS ((1 << X86_EXC_BP) | (1 << X86_EXC_OF))

/* Bitmap of exceptions which are classified as aborts. */
#define X86_EXC_ABORTS ((1 << X86_EXC_DF) | (1 << X86_EXC_MC))

/* Number of reserved vectors for exceptions. */
#define X86_NR_RESERVED_VECTORS 32

/*
 * Error Code mnemonics.
 */
/* Segment-based Error Code - architecturally defined. */
#define X86_EC_EXT (1U << 0) /* External event. */
#define X86_EC_IDT (1U << 1) /* Descriptor Location.  IDT, or LDT/GDT */
#define X86_EC_TI  (1U << 2) /* Only if !IDT.  LDT or GDT. */

/* Segment-based Error Code - supplemental constants. */
#define X86_EC_TABLE_MASK (3  << 1)
#define X86_EC_SEL_SHIFT  3
#define X86_EC_SEL_MASK   (~0U << X86_EC_SEL_SHIFT)
#define X86_EC_GDT        0
#define X86_EC_LDT        X86_EC_TI

/* Pagefault Error Code - architecturally defined. */
#define X86_PFEC_PRESENT  (1U << 0)
#define X86_PFEC_WRITE    (1U << 1)
#define X86_PFEC_USER     (1U << 2)
#define X86_PFEC_RSVD     (1U << 3)
#define X86_PFEC_INSN     (1U << 4)
#define X86_PFEC_PK       (1U << 5)

/* Pagefault Error Code - Short form mnemonics. */
#define X86_PFEC_P X86_PFEC_PRESENT
#define X86_PFEC_W X86_PFEC_WRITE
#define X86_PFEC_U X86_PFEC_USER
#define X86_PFEC_R X86_PFEC_RSVD
#define X86_PFEC_I X86_PFEC_INSN
#define X86_PFEC_K X86_PFEC_PK

/*
 * Selector mnemonics.
 */
/* Architecturally defined. */
#define X86_SEL_TI        (1U << 2) /* Table Indicator. */

/* Supplemental constants. */
#define X86_SEL_RPL_MASK  3         /* RPL is the bottom two bits. */
#define X86_SEL_GDT       0
#define X86_SEL_LDT       X86_SEL_TI

#endif /* XTF_X86_PROCESSOR_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
