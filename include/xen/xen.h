/*
 * Xen public hypercall interface
 */

#ifndef XEN_PUBLIC_XEN_H
#define XEN_PUBLIC_XEN_H

#if defined(__i386__) || defined(__x86_64__)
#include "arch-x86/xen.h"
#elif defined(__arm__) || defined (__aarch64__)
#include "arch-arm.h"
#else
#error Bad architecture
#endif

#define __HYPERVISOR_set_trap_table        0
#define __HYPERVISOR_mmu_update            1
#define __HYPERVISOR_set_gdt               2
#define __HYPERVISOR_stack_switch          3
#define __HYPERVISOR_set_callbacks         4
#define __HYPERVISOR_fpu_taskswitch        5
#define __HYPERVISOR_sched_op_compat       6 /* compat since 0x00030101 */
#define __HYPERVISOR_platform_op           7
#define __HYPERVISOR_set_debugreg          8
#define __HYPERVISOR_get_debugreg          9
#define __HYPERVISOR_update_descriptor    10
#define __HYPERVISOR_memory_op            12
#define __HYPERVISOR_multicall            13
#define __HYPERVISOR_update_va_mapping    14
#define __HYPERVISOR_set_timer_op         15
#define __HYPERVISOR_event_channel_op_compat 16 /* compat since 0x00030202 */
#define __HYPERVISOR_xen_version          17
#define __HYPERVISOR_console_io           18
#define __HYPERVISOR_physdev_op_compat    19 /* compat since 0x00030202 */
#define __HYPERVISOR_grant_table_op       20
#define __HYPERVISOR_vm_assist            21
#define __HYPERVISOR_update_va_mapping_otherdomain 22
#define __HYPERVISOR_iret                 23 /* x86 only */
#define __HYPERVISOR_vcpu_op              24
#define __HYPERVISOR_set_segment_base     25 /* x86/64 only */
#define __HYPERVISOR_mmuext_op            26
#define __HYPERVISOR_xsm_op               27
#define __HYPERVISOR_nmi_op               28
#define __HYPERVISOR_sched_op             29
#define __HYPERVISOR_callback_op          30
#define __HYPERVISOR_xenoprof_op          31
#define __HYPERVISOR_event_channel_op     32
#define __HYPERVISOR_physdev_op           33
#define __HYPERVISOR_hvm_op               34
#define __HYPERVISOR_sysctl               35
#define __HYPERVISOR_domctl               36
#define __HYPERVISOR_kexec_op             37
#define __HYPERVISOR_tmem_op              38
#define __HYPERVISOR_argo_op              39
#define __HYPERVISOR_xenpmu_op            40

/* Architecture-specific hypercall definitions. */
#define __HYPERVISOR_arch_0               48
#define __HYPERVISOR_arch_1               49
#define __HYPERVISOR_arch_2               50
#define __HYPERVISOR_arch_3               51
#define __HYPERVISOR_arch_4               52
#define __HYPERVISOR_arch_5               53
#define __HYPERVISOR_arch_6               54
#define __HYPERVISOR_arch_7               55

#ifndef __ASSEMBLY__
typedef uint16_t domid_t;
#endif

#define DOMID_FIRST_RESERVED (0x7ff0U)
#define DOMID_SELF (0x7ff0U)

/* Commands to HYPERVISOR_console_io */
#define CONSOLEIO_write                   0

/*
 * Commands to HYPERVISOR_vm_assist().
 */
#define VMASST_CMD_enable                 0
#define VMASST_CMD_disable                1

/*
 * x86 guests: Sane behaviour for virtual iopl
 *  - virtual iopl updated from do_iret() hypercalls.
 *  - virtual iopl reported in bounce frames.
 *  - guest kernels assumed to be level 0 for the purpose of iopl checks.
 */
#define VMASST_TYPE_architectural_iopl   4


#ifndef __ASSEMBLY__
struct vcpu_time_info {
    /*
     * Updates to the following values are preceded and followed by an
     * increment of 'version'. The guest can therefore detect updates by
     * looking for changes to 'version'. If the least-significant bit of
     * the version number is set then an update is in progress and the guest
     * must wait to read a consistent set of values.
     * The correct way to interact with the version number is similar to
     * Linux's seqlock: see the implementations of read_seqbegin/read_seqretry.
     */
    uint32_t version;
    uint32_t pad0;
    uint64_t tsc_timestamp;   /* TSC at last update of time vals.  */
    uint64_t system_time;     /* Time, in nanosecs, since boot.    */
    /*
     * Current system time:
     *   system_time +
     *   ((((tsc - tsc_timestamp) << tsc_shift) * tsc_to_system_mul) >> 32)
     * CPU frequency (Hz):
     *   ((10^9 << 32) / tsc_to_system_mul) >> tsc_shift
     */
    uint32_t tsc_to_system_mul;
    int8_t   tsc_shift;
    uint8_t  flags;
    uint8_t  pad1[2];
}; /* 32 bytes */
typedef struct vcpu_time_info vcpu_time_info_t;

struct vcpu_info {
    /*
     * 'evtchn_upcall_pending' is written non-zero by Xen to indicate
     * a pending notification for a particular VCPU. It is then cleared
     * by the guest OS /before/ checking for pending work, thus avoiding
     * a set-and-check race. Note that the mask is only accessed by Xen
     * on the CPU that is currently hosting the VCPU. This means that the
     * pending and mask flags can be updated by the guest without special
     * synchronisation (i.e., no need for the x86 LOCK prefix).
     * This may seem suboptimal because if the pending flag is set by
     * a different CPU then an IPI may be scheduled even when the mask
     * is set. However, note:
     *  1. The task of 'interrupt holdoff' is covered by the per-event-
     *     channel mask bits. A 'noisy' event that is continually being
     *     triggered can be masked at source at this very precise
     *     granularity.
     *  2. The main purpose of the per-VCPU mask is therefore to restrict
     *     reentrant execution: whether for concurrency control, or to
     *     prevent unbounded stack usage. Whatever the purpose, we expect
     *     that the mask will be asserted only for short periods at a time,
     *     and so the likelihood of a 'spurious' IPI is suitably small.
     * The mask is read before making an event upcall to the guest: a
     * non-zero mask therefore guarantees that the VCPU will not receive
     * an upcall activation. The mask is cleared when the VCPU requests
     * to block: this avoids wakeup-waiting races.
     */
    uint8_t evtchn_upcall_pending;
    uint8_t evtchn_upcall_mask;
    unsigned long evtchn_pending_sel;
    struct arch_vcpu_info arch;
    struct vcpu_time_info time;
}; /* 64 bytes (x86) */

struct shared_info {
    struct vcpu_info vcpu_info[32];

    /*
     * A domain can create "event channels" on which it can send and receive
     * asynchronous event notifications. There are three classes of event that
     * are delivered by this mechanism:
     *  1. Bi-directional inter- and intra-domain connections. Domains must
     *     arrange out-of-band to set up a connection (usually by allocating
     *     an unbound 'listener' port and avertising that via a storage service
     *     such as xenstore).
     *  2. Physical interrupts. A domain with suitable hardware-access
     *     privileges can bind an event-channel port to a physical interrupt
     *     source.
     *  3. Virtual interrupts ('events'). A domain can bind an event-channel
     *     port to a virtual interrupt source, such as the virtual-timer
     *     device or the emergency console.
     *
     * Event channels are addressed by a "port index". Each channel is
     * associated with two bits of information:
     *  1. PENDING -- notifies the domain that there is a pending notification
     *     to be processed. This bit is cleared by the guest.
     *  2. MASK -- if this bit is clear then a 0->1 transition of PENDING
     *     will cause an asynchronous upcall to be scheduled. This bit is only
     *     updated by the guest. It is read-only within Xen. If a channel
     *     becomes pending while the channel is masked then the 'edge' is lost
     *     (i.e., when the channel is unmasked, the guest must manually handle
     *     pending notifications as no upcall will be scheduled by Xen).
     *
     * To expedite scanning of pending notifications, any 0->1 pending
     * transition on an unmasked channel causes a corresponding bit in a
     * per-vcpu selector word to be set. Each bit in the selector covers a
     * 'C long' in the PENDING bitfield array.
     */
    unsigned long evtchn_pending[sizeof(unsigned long) * 8];
    unsigned long evtchn_mask[sizeof(unsigned long) * 8];

    /*
     * Wallclock time: updated only by control software. Guests should base
     * their gettimeofday() syscall on this wallclock-base value.
     */
    uint32_t wc_version;      /* Version counter: see vcpu_time_info_t. */
    uint32_t wc_sec;          /* Secs  00:00:00 UTC, Jan 1, 1970.  */
    uint32_t wc_nsec;         /* Nsecs 00:00:00 UTC, Jan 1, 1970.  */
#if !defined(__i386__)
    uint32_t wc_sec_hi;
#endif

    struct arch_shared_info arch;
};
typedef struct shared_info shared_info_t;

struct xen_pv_start_info {
    /* THE FOLLOWING ARE FILLED IN BOTH ON INITIAL BOOT AND ON RESUME.    */
    char magic[32];             /* "xen-<version>-<platform>".            */
    unsigned long nr_pages;     /* Total pages allocated to this domain.  */
    unsigned long shared_info;  /* MACHINE address of shared info struct. */
    uint32_t flags;             /* SIF_xxx flags.                         */
    xen_pfn_t store_mfn;        /* MACHINE page number of shared page.    */
    uint32_t store_evtchn;      /* Event channel for store communication. */
    union {
        struct {
            xen_pfn_t mfn;      /* MACHINE page number of console page.   */
            uint32_t  evtchn;   /* Event channel for console page.        */
        } domU;
        struct {
            uint32_t info_off;  /* Offset of console_info struct.         */
            uint32_t info_size; /* Size of console_info struct from start.*/
        } dom0;
    } console;
    /* THE FOLLOWING ARE ONLY FILLED IN ON INITIAL BOOT (NOT RESUME).     */
    unsigned long pt_base;      /* VIRTUAL address of page directory.     */
    unsigned long nr_pt_frames; /* Number of bootstrap p.t. frames.       */
    unsigned long mfn_list;     /* VIRTUAL address of page-frame list.    */
    unsigned long mod_start;    /* VIRTUAL address of pre-loaded module   */
                                /* (PFN of pre-loaded module if           */
                                /*  SIF_MOD_START_PFN set in flags).      */
    unsigned long mod_len;      /* Size (bytes) of pre-loaded module.     */
#define MAX_GUEST_CMDLINE 1024
    int8_t cmd_line[MAX_GUEST_CMDLINE];
    /* The pfn range here covers both page table and p->m table frames.   */
    unsigned long first_p2m_pfn;/* 1st pfn forming initial P->M table.    */
    unsigned long nr_p2m_frames;/* # of pfns forming initial P->M table.  */
};
typedef struct xen_pv_start_info xen_pv_start_info_t;
#endif

/* These flags are passed in the 'flags' field of start_info_t. */
#define SIF_PRIVILEGED    (1u << 0)  /* Is the domain privileged? */
#define SIF_INITDOMAIN    (1u << 1)  /* Is this the initial control domain? */

/* MMU UPDATE operations */
#ifndef __ASSEMBLY__
struct mmu_update {
#define MMU_NORMAL_PT_UPDATE      0 /* checked '*ptr = val'. ptr is MA.      */
#define MMU_MACHPHYS_UPDATE       1 /* ptr = MA of frame to modify entry for */
#define MMU_PT_UPDATE_PRESERVE_AD 2 /* atomically: *ptr = val | (*ptr&(A|D)) */
    uint64_t ptr;       /* Machine address of PTE. */
    uint64_t val;       /* New contents of PTE.    */
};
typedef struct mmu_update mmu_update_t;

/*
 * ` enum neg_errnoval
 * ` HYPERVISOR_multicall(multicall_entry_t call_list[],
 * `                      uint32_t nr_calls);
 *
 * NB. The fields are logically the natural register size for this
 * architecture. In cases where xen_ulong_t is larger than this then
 * any unused bits in the upper portion must be zero.
 */
struct multicall_entry {
    unsigned long op, result;
    unsigned long args[6];
};
typedef struct multicall_entry multicall_entry_t;
#endif

/*
 * MMU EXTENDED OPERATIONS
 *
 * HYPERVISOR_mmuext_op(mmuext_op_t uops[],
 *                      unsigned int count,
 *                      unsigned int *pdone,
 *                      unsigned int foreigndom)
 *
 * HYPERVISOR_mmuext_op() accepts a list of mmuext_op structures.
 * A foreigndom (FD) can be specified (or DOMID_SELF for none).
 * Where the FD has some effect, it is described below.
 *
 * cmd: MMUEXT_(UN)PIN_*_TABLE
 * mfn: Machine frame number to be (un)pinned as a p.t. page.
 *      The frame must belong to the FD, if one is specified.
 *
 * cmd: MMUEXT_NEW_BASEPTR
 * mfn: Machine frame number of new page-table base to install in MMU.
 *
 * cmd: MMUEXT_NEW_USER_BASEPTR [x86/64 only]
 * mfn: Machine frame number of new page-table base to install in MMU
 *      when in user space.
 *
 * cmd: MMUEXT_TLB_FLUSH_LOCAL
 * No additional arguments. Flushes local TLB.
 *
 * cmd: MMUEXT_INVLPG_LOCAL
 * linear_addr: Linear address to be flushed from the local TLB.
 *
 * cmd: MMUEXT_TLB_FLUSH_MULTI
 * vcpumask: Pointer to bitmap of VCPUs to be flushed.
 *
 * cmd: MMUEXT_INVLPG_MULTI
 * linear_addr: Linear address to be flushed.
 * vcpumask: Pointer to bitmap of VCPUs to be flushed.
 *
 * cmd: MMUEXT_TLB_FLUSH_ALL
 * No additional arguments. Flushes all VCPUs' TLBs.
 *
 * cmd: MMUEXT_INVLPG_ALL
 * linear_addr: Linear address to be flushed from all VCPUs' TLBs.
 *
 * cmd: MMUEXT_FLUSH_CACHE
 * No additional arguments. Writes back and flushes cache contents.
 *
 * cmd: MMUEXT_FLUSH_CACHE_GLOBAL
 * No additional arguments. Writes back and flushes cache contents
 * on all CPUs in the system.
 *
 * cmd: MMUEXT_SET_LDT
 * linear_addr: Linear address of LDT base (NB. must be page-aligned).
 * nr_ents: Number of entries in LDT.
 *
 * cmd: MMUEXT_CLEAR_PAGE
 * mfn: Machine frame number to be cleared.
 *
 * cmd: MMUEXT_COPY_PAGE
 * mfn: Machine frame number of the destination page.
 * src_mfn: Machine frame number of the source page.
 *
 * cmd: MMUEXT_[UN]MARK_SUPER
 * mfn: Machine frame number of head of superpage to be [un]marked.
 */
#define MMUEXT_PIN_L1_TABLE      0
#define MMUEXT_PIN_L2_TABLE      1
#define MMUEXT_PIN_L3_TABLE      2
#define MMUEXT_PIN_L4_TABLE      3
#define MMUEXT_UNPIN_TABLE       4
#define MMUEXT_NEW_BASEPTR       5
#define MMUEXT_TLB_FLUSH_LOCAL   6
#define MMUEXT_INVLPG_LOCAL      7
#define MMUEXT_TLB_FLUSH_MULTI   8
#define MMUEXT_INVLPG_MULTI      9
#define MMUEXT_TLB_FLUSH_ALL    10
#define MMUEXT_INVLPG_ALL       11
#define MMUEXT_FLUSH_CACHE      12
#define MMUEXT_SET_LDT          13
#define MMUEXT_NEW_USER_BASEPTR 15
#define MMUEXT_CLEAR_PAGE       16
#define MMUEXT_COPY_PAGE        17
#define MMUEXT_FLUSH_CACHE_GLOBAL 18
#define MMUEXT_MARK_SUPER       19
#define MMUEXT_UNMARK_SUPER     20

#ifndef __ASSEMBLY__
struct mmuext_op {
    unsigned int cmd; /* => enum mmuext_cmd */
    union {
        /* [UN]PIN_TABLE, NEW_BASEPTR, NEW_USER_BASEPTR
         * CLEAR_PAGE, COPY_PAGE, [UN]MARK_SUPER */
        xen_pfn_t     mfn;
        /* INVLPG_LOCAL, INVLPG_ALL, SET_LDT */
        unsigned long linear_addr;
    } arg1;
    union {
        /* SET_LDT */
        unsigned int nr_ents;
        /* TLB_FLUSH_MULTI, INVLPG_MULTI */
        const void *vcpumask;
        /* COPY_PAGE */
        xen_pfn_t src_mfn;
    } arg2;
};
typedef struct mmuext_op mmuext_op_t;
#endif

/* These are passed as 'flags' to update_va_mapping. They can be ORed. */
/* When specifying UVMF_MULTI, also OR in a pointer to a CPU bitmap.   */
/* UVMF_LOCAL is merely UVMF_MULTI with a NULL bitmap pointer.         */
#ifndef __ASSEMBLY__
enum XEN_UVMF {
    UVMF_NONE             = 0 << 0, /* No flushing at all.   */
    UVMF_TLB_FLUSH        = 1 << 0, /* Flush entire TLB(s).  */
    UVMF_INVLPG           = 2 << 0, /* Flush only one entry. */
    UVMF_FLUSHTYPE_MASK   = 3 << 0,
    UVMF_MULTI            = 0 << 2, /* Flush subset of TLBs. */
    UVMF_LOCAL            = 0 << 2, /* Flush local TLB.      */
    UVMF_ALL              = 1 << 2, /* Flush all TLBs.       */
};
#endif

#ifndef __ASSEMBLY__
typedef struct {
    union {
        void *p;
        uint64_t __aligned(8) q;
    };
} guest_handle_64_t;
#endif

#endif /* XEN_PUBLIC_XEN_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
