/*
 * Xen x86 public interface
 */

#ifndef XEN_PUBLIC_ARCH_X86_XEN_H
#define XEN_PUBLIC_ARCH_X86_XEN_H

#if defined (__i386__)
# include "xen-x86_32.h"
#elif defined (__x86_64__)
# include "xen-x86_64.h"
#else
# error Bad architecture
#endif

#include "cpuid.h"

#ifndef __ASSEMBLY__
typedef unsigned long xen_pfn_t;

/*
 * Send an array of these to HYPERVISOR_set_trap_table().
 * Terminate the array with a sentinel entry, with traps[].address==0.
 * The privilege level specifies which modes may enter a trap via a software
 * interrupt. On x86/64, since rings 1 and 2 are unavailable, we allocate
 * privilege levels as follows:
 *  Level == 0: Noone may enter
 *  Level == 1: Kernel may enter
 *  Level == 2: Kernel may enter
 *  Level == 3: Everyone may enter
 */
struct xen_trap_info {
    uint8_t       vector;  /* exception vector                              */
    uint8_t       flags;   /* 0-3: privilege level; 4: clear event enable?  */
    uint16_t      cs;      /* code selector                                 */
    unsigned long address; /* code offset                                   */
};

/*
 * The following is all CPU context. Note that the fpu_ctxt block is filled
 * in by FXSAVE if the CPU has feature FXSR; otherwise FSAVE is used.
 */
struct xen_vcpu_guest_context {
    /* FPU registers come first so they can be aligned for FXSAVE/FXRSTOR.  */
    struct { char x[512]; } fpu_ctxt;       /* User-level FPU registers     */
#define VGCF_I387_VALID                (1<<0)
#define VGCF_IN_KERNEL                 (1<<2)
#define _VGCF_i387_valid               0
#define VGCF_i387_valid                (1<<_VGCF_i387_valid)
#define _VGCF_in_kernel                2
#define VGCF_in_kernel                 (1<<_VGCF_in_kernel)
#define _VGCF_failsafe_disables_events 3
#define VGCF_failsafe_disables_events  (1<<_VGCF_failsafe_disables_events)
#define _VGCF_syscall_disables_events  4
#define VGCF_syscall_disables_events   (1<<_VGCF_syscall_disables_events)
#define _VGCF_online                   5
#define VGCF_online                    (1<<_VGCF_online)
    unsigned long flags;                    /* VGCF_* flags                 */
    struct xen_cpu_user_regs user_regs;     /* User-level CPU registers     */
    struct xen_trap_info trap_ctxt[256];    /* Virtual IDT                  */
    unsigned long ldt_base, ldt_ents;       /* LDT (linear address, # ents) */
    unsigned long gdt_frames[16], gdt_ents; /* GDT (machine frames, # ents) */
    unsigned long kernel_ss, kernel_sp;     /* Virtual TSS (only SS1/SP1)   */
    /* NB. User pagetable on x86/64 is placed in ctrlreg[1]. */
    unsigned long ctrlreg[8];               /* CR0-CR7 (control registers)  */
    unsigned long debugreg[8];              /* DB0-DB7 (debug registers)    */
#ifdef __i386__
    unsigned long event_callback_cs;        /* CS:EIP of event callback     */
    unsigned long event_callback_eip;
    unsigned long failsafe_callback_cs;     /* CS:EIP of failsafe callback  */
    unsigned long failsafe_callback_eip;
#else
    unsigned long event_callback_eip;
    unsigned long failsafe_callback_eip;
    union {
        unsigned long syscall_callback_eip;
        struct {
            unsigned int event_callback_cs;    /* compat CS of event cb     */
            unsigned int failsafe_callback_cs; /* compat CS of failsafe cb  */
        };
    };
#endif
    unsigned long vm_assist;                /* VMASST_TYPE_* bitmap */
#ifdef __x86_64__
    /* Segment base addresses. */
    uint64_t      fs_base;
    uint64_t      gs_base_kernel;
    uint64_t      gs_base_user;
#endif
};
typedef struct xen_vcpu_guest_context xen_vcpu_guest_context_t;

struct arch_shared_info {
    /*
     * Number of valid entries in the p2m table(s) anchored at
     * pfn_to_mfn_frame_list_list and/or p2m_vaddr.
     */
    unsigned long max_pfn;
    /*
     * Frame containing list of mfns containing list of mfns containing p2m.
     * A value of 0 indicates it has not yet been set up, ~0 indicates it has
     * been set to invalid e.g. due to the p2m being too large for the 3-level
     * p2m tree. In this case the linear mapper p2m list anchored at p2m_vaddr
     * is to be used.
     */
    xen_pfn_t     pfn_to_mfn_frame_list_list;
    unsigned long nmi_reason;
    /*
     * Following three fields are valid if p2m_cr3 contains a value different
     * from 0.
     * p2m_cr3 is the root of the address space where p2m_vaddr is valid.
     * p2m_cr3 is in the same format as a cr3 value in the vcpu register state
     * and holds the folded machine frame number (via xen_pfn_to_cr3) of a
     * L3 or L4 page table.
     * p2m_vaddr holds the virtual address of the linear p2m list. All entries
     * in the range [0...max_pfn[ are accessible via this pointer.
     * p2m_generation will be incremented by the guest before and after each
     * change of the mappings of the p2m list. p2m_generation starts at 0 and
     * a value with the least significant bit set indicates that a mapping
     * update is in progress. This allows guest external software (e.g. in Dom0)
     * to verify that read mappings are consistent and whether they have changed
     * since the last check.
     * Modifying a p2m element in the linear p2m list is allowed via an atomic
     * write only.
     */
    unsigned long p2m_cr3;         /* cr3 value of the p2m address space */
    unsigned long p2m_vaddr;       /* virtual address of the p2m list */
    unsigned long p2m_generation;  /* generation count of p2m mapping */
#ifdef __i386__
    /* There's no room for this field in the generic structure. */
    uint32_t wc_sec_hi;
#endif
};

/*
 * struct xen_arch_domainconfig's ABI is covered by
 * XEN_DOMCTL_INTERFACE_VERSION.
 */
struct xen_arch_domainconfig {
#define _XEN_X86_EMU_LAPIC          0
#define XEN_X86_EMU_LAPIC           (1U<<_XEN_X86_EMU_LAPIC)
#define _XEN_X86_EMU_HPET           1
#define XEN_X86_EMU_HPET            (1U<<_XEN_X86_EMU_HPET)
#define _XEN_X86_EMU_PM             2
#define XEN_X86_EMU_PM              (1U<<_XEN_X86_EMU_PM)
#define _XEN_X86_EMU_RTC            3
#define XEN_X86_EMU_RTC             (1U<<_XEN_X86_EMU_RTC)
#define _XEN_X86_EMU_IOAPIC         4
#define XEN_X86_EMU_IOAPIC          (1U<<_XEN_X86_EMU_IOAPIC)
#define _XEN_X86_EMU_PIC            5
#define XEN_X86_EMU_PIC             (1U<<_XEN_X86_EMU_PIC)
#define _XEN_X86_EMU_VGA            6
#define XEN_X86_EMU_VGA             (1U<<_XEN_X86_EMU_VGA)
#define _XEN_X86_EMU_IOMMU          7
#define XEN_X86_EMU_IOMMU           (1U<<_XEN_X86_EMU_IOMMU)
#define _XEN_X86_EMU_PIT            8
#define XEN_X86_EMU_PIT             (1U<<_XEN_X86_EMU_PIT)
#define _XEN_X86_EMU_USE_PIRQ       9
#define XEN_X86_EMU_USE_PIRQ        (1U<<_XEN_X86_EMU_USE_PIRQ)
#define _XEN_X86_EMU_VPCI           10
#define XEN_X86_EMU_VPCI            (1U<<_XEN_X86_EMU_VPCI)

#define XEN_X86_EMU_ALL             (XEN_X86_EMU_LAPIC | XEN_X86_EMU_HPET |  \
                                     XEN_X86_EMU_PM | XEN_X86_EMU_RTC |      \
                                     XEN_X86_EMU_IOAPIC | XEN_X86_EMU_PIC |  \
                                     XEN_X86_EMU_VGA | XEN_X86_EMU_IOMMU |   \
                                     XEN_X86_EMU_PIT | XEN_X86_EMU_USE_PIRQ |\
                                     XEN_X86_EMU_VPCI)
    uint32_t emulation_flags;
};

#endif

/**
 * Xen Forced Emulation Prefix.
 *
 * For PV guests, may prefix a `cpuid` instruction to allow Xen to fill in
 * information, rather than reading the hardware values.
 *
 * For HVM guests (and with the Xen 'hvm_fep' command line option enabled),
 * may be used for cause any arbitrary instruction to be emulated in the
 * hypervisor's x86 emulator.
 */
#ifdef __ASSEMBLY__
#define _ASM_XEN_FEP ud2a; .ascii "xen";
#else
#define _ASM_XEN_FEP "ud2a; .ascii \"xen\";"
#define _BIN_XEN_FEP "\x0f\x0bxen"
#endif

#endif /* XEN_PUBLIC_ARCH_X86_XEN_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
