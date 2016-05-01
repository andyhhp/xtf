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
