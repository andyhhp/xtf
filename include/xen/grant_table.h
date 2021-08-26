/*
 * Xen public Grant Table hypercall interface
 */

#ifndef XEN_PUBLIC_GRANT_TABLE_H
#define XEN_PUBLIC_GRANT_TABLE_H

#include "xen.h"

#define GNTST_okay              (0)  /* Normal return.                        */
#define GNTST_general_error     (-1) /* General undefined error.              */
#define GNTST_bad_domain        (-2) /* Unrecognsed domain id.                */
#define GNTST_bad_gntref        (-3) /* Unrecognised or inappropriate gntref. */
#define GNTST_bad_handle        (-4) /* Unrecognised or inappropriate handle. */
#define GNTST_bad_virt_addr     (-5) /* Inappropriate virtual address to map. */
#define GNTST_bad_dev_addr      (-6) /* Inappropriate device address to unmap.*/
#define GNTST_no_device_space   (-7) /* Out of space in I/O MMU.              */
#define GNTST_permission_denied (-8) /* Not enough privilege for operation.   */
#define GNTST_bad_page          (-9) /* Specified page was invalid for op.    */
#define GNTST_bad_copy_arg     (-10) /* copy arguments cross page boundary.   */
#define GNTST_address_too_big  (-11) /* transfer page address too large.      */
#define GNTST_eagain           (-12) /* Operation not done; try again.        */

#define GNTTABOP_error_msgs {                       \
        "okay",                                     \
        "undefined error",                          \
        "unrecognised domain id",                   \
        "invalid grant reference",                  \
        "invalid mapping handle",                   \
        "invalid virtual address",                  \
        "invalid device address",                   \
        "no spare translation slot in the I/O MMU", \
        "permission denied",                        \
        "bad page",                                 \
        "copy arguments cross page boundary",       \
        "page address size too large",              \
        "operation not done; try again",            \
    }

/*
 * Type of grant entry.
 *  GTF_invalid: This grant entry grants no privileges.
 *  GTF_permit_access: Allow @domid to map/access @frame.
 *  GTF_accept_transfer: Allow @domid to transfer ownership of one page frame
 *                       to this guest. Xen writes the page number to @frame.
 *  GTF_transitive: Allow @domid to transitively access a subrange of
 *                  @trans_grant in @trans_domid.  No mappings are allowed.
 */
#define GTF_invalid         (0 << 0)
#define GTF_permit_access   (1 << 0)
#define GTF_accept_transfer (2 << 0)
#define GTF_transitive      (3 << 0)
#define GTF_type_mask       (3 << 0)

/*
 * Subflags for GTF_permit_access.
 *  GTF_readonly: Restrict @domid to read-only mappings and accesses. [GST]
 *  GTF_reading: Grant entry is currently mapped for reading by @domid. [XEN]
 *  GTF_writing: Grant entry is currently mapped for writing by @domid. [XEN]
 *  GTF_PAT, GTF_PWT, GTF_PCD: (x86) cache attribute flags for the grant [GST]
 *  GTF_sub_page: Grant access to only a subrange of the page.  @domid
 *                will only be allowed to copy from the grant, and not
 *                map it. [GST]
 */
#define _GTF_readonly       2
#define GTF_readonly        (1 << _GTF_readonly)

#define _GTF_reading        3
#define GTF_reading         (1 << _GTF_reading)

#define _GTF_writing        4
#define GTF_writing         (1 << _GTF_writing)

#define _GTF_PWT            5
#define GTF_PWT             (1 << _GTF_PWT)

#define _GTF_PCD            6
#define GTF_PCD             (1 << _GTF_PCD)

#define _GTF_PAT            7
#define GTF_PAT             (1 << _GTF_PAT)

#define _GTF_sub_page       8
#define GTF_sub_page        (1 << _GTF_sub_page)

/*
 * Subflags for GTF_accept_transfer:
 *  GTF_transfer_committed: Xen sets this flag to indicate that it is committed
 *      to transferring ownership of a page frame. When a guest sees this flag
 *      it must /not/ modify the grant entry until GTF_transfer_completed is
 *      set by Xen.
 *  GTF_transfer_completed: It is safe for the guest to spin-wait on this flag
 *      after reading GTF_transfer_committed. Xen will always write the frame
 *      address, followed by ORing this flag, in a timely manner.
 */
#define _GTF_transfer_committed 2
#define GTF_transfer_committed  (1 << _GTF_transfer_committed)

#define _GTF_transfer_completed 3
#define GTF_transfer_completed  (1 << _GTF_transfer_completed)

typedef uint32_t grant_ref_t;
typedef uint32_t grant_handle_t;

typedef struct
{
    /* GTF_xxx: various type and flag information.  [XEN,GST] */
    uint16_t flags;
    /* The domain being granted foreign privileges. [GST] */
    domid_t  domid;
    /*
     * GTF_permit_access: GFN that @domid is allowed to map and access. [GST]
     * GTF_accept_transfer: GFN that @domid is allowed to transfer into. [GST]
     * GTF_transfer_completed: MFN whose ownership transferred by @domid
     *                         (non-translated guests only). [XEN]
     */
    uint32_t frame;
} grant_entry_v1_t;

typedef struct {
    uint16_t flags;
    domid_t  domid;
} grant_entry_header_t;

typedef union {
    grant_entry_header_t hdr;

    /*
     * This member is used for V1-style full page grants, where either:
     *
     * -- hdr.type is GTF_accept_transfer, or
     * -- hdr.type is GTF_permit_access and GTF_sub_page is not set.
     *
     * In that case, the frame field has the same semantics as the
     * field of the same name in the V1 entry structure.
     */
    struct {
        grant_entry_header_t hdr;
        uint32_t pad0;
        uint64_t frame;
    } full_page;

    /*
     * If the grant type is GTF_grant_access and GTF_sub_page is set,
     * @domid is allowed to access bytes [@page_off,@page_off+@length)
     * in frame @frame.
     */
    struct {
        grant_entry_header_t hdr;
        uint16_t page_off;
        uint16_t length;
        uint64_t frame;
    } sub_page;

    /*
     * If the grant is GTF_transitive, @domid is allowed to use the
     * grant @gref in domain @trans_domid, as if it was the local
     * domain.  Obviously, the transitive access must be compatible
     * with the original grant.
     *
     * The current version of Xen does not allow transitive grants
     * to be mapped.
     */
    struct {
        grant_entry_header_t hdr;
        domid_t trans_domid;
        uint16_t pad0;
        grant_ref_t gref;
    } transitive;

    uint32_t __spacer[4]; /* Pad to a power of two */
} grant_entry_v2_t;

/* Map the grant entry for access by I/O devices. */
#define _GNTMAP_device_map      0
#define GNTMAP_device_map       (1 << _GNTMAP_device_map)
/* Map the grant entry for access by host CPUs. */
#define _GNTMAP_host_map        1
#define GNTMAP_host_map         (1 << _GNTMAP_host_map)
/* Accesses to the granted frame will be restricted to read-only access. */
#define _GNTMAP_readonly        2
#define GNTMAP_readonly         (1 << _GNTMAP_readonly)
/*
 * GNTMAP_host_map subflag:
 *  0 => The host mapping is usable only by the guest OS.
 *  1 => The host mapping is usable by guest OS + current application.
 */
#define _GNTMAP_application_map 3
#define GNTMAP_application_map  (1 << _GNTMAP_application_map)

/*
 * GNTMAP_contains_pte subflag:
 *  0 => This map request contains a host virtual address.
 *  1 => This map request contains the machine addess of the PTE to update.
 */
#define _GNTMAP_contains_pte    4
#define GNTMAP_contains_pte     (1 << _GNTMAP_contains_pte)

/*
 * Bits to be placed in guest kernel available PTE bits (architecture
 * dependent; only supported when XENFEAT_gnttab_map_avail_bits is set).
 */
#define _GNTMAP_guest_avail0    16
#define GNTMAP_guest_avail_mask ((uint32_t)~0 << _GNTMAP_guest_avail0)

/*
 * GNTTABOP_map_grant_ref: Map the grant entry (<dom>,<ref>) for access
 * by devices and/or host CPUs. If successful, <handle> is a tracking number
 * that must be presented later to destroy the mapping(s). On error, <status>
 * is a negative status code.
 * NOTES:
 *  1. If GNTMAP_device_map is specified then <dev_bus_addr> is the address
 *     via which I/O devices may access the granted frame.
 *  2. If GNTMAP_host_map is specified then a mapping will be added at
 *     either a host virtual address in the current address space, or at
 *     a PTE at the specified machine address.  The type of mapping to
 *     perform is selected through the GNTMAP_contains_pte flag, and the
 *     address is specified in <host_addr>.
 *  3. Mappings should only be destroyed via GNTTABOP_unmap_grant_ref. If a
 *     host mapping is destroyed by other means then it is *NOT* guaranteed
 *     to be accounted to the correct grant reference!
 */
#define GNTTABOP_map_grant_ref        0
struct gnttab_map_grant_ref {
    /* IN parameters. */
    uint64_t host_addr;
    uint32_t flags;               /* GNTMAP_* */
    grant_ref_t ref;
    domid_t  dom;
    /* OUT parameters. */
    int16_t  status;              /* => enum grant_status */
    grant_handle_t handle;
    uint64_t dev_bus_addr;
};

/*
 * GNTTABOP_unmap_grant_ref: Destroy one or more grant-reference mappings
 * tracked by <handle>. If <host_addr> or <dev_bus_addr> is zero, that
 * field is ignored. If non-zero, they must refer to a device/host mapping
 * that is tracked by <handle>
 * NOTES:
 *  1. The call may fail in an undefined manner if either mapping is not
 *     tracked by <handle>.
 *  3. After executing a batch of unmaps, it is guaranteed that no stale
 *     mappings will remain in the device or host TLBs.
 */
#define GNTTABOP_unmap_grant_ref      1
struct gnttab_unmap_grant_ref {
    /* IN parameters. */
    uint64_t host_addr;
    uint64_t dev_bus_addr;
    grant_handle_t handle;
    /* OUT parameters. */
    int16_t  status;              /* => enum grant_status */
};

/*
 * GNTTABOP_setup_table: Set up a grant table for <dom> comprising at least
 * <nr_frames> pages. The frame addresses are written to the <frame_list>.
 * Only <nr_frames> addresses are written, even if the table is larger.
 * NOTES:
 *  1. <dom> may be specified as DOMID_SELF.
 *  2. Only a sufficiently-privileged domain may specify <dom> != DOMID_SELF.
 *  3. Xen may not support more than a single grant-table page per domain.
 */
#define GNTTABOP_setup_table          2
struct gnttab_setup_table
{
    /* IN parameters. */
    domid_t  dom;
    uint32_t nr_frames;
    /* OUT parameters. */
    int16_t  status;
    unsigned long *frame_list;
};

/*
 * GNTTABOP_unmap_and_replace: Destroy one or more grant-reference mappings
 * tracked by <handle> but atomically replace the page table entry with one
 * pointing to the machine address under <new_addr>.  <new_addr> will be
 * redirected to the null entry.
 * NOTES:
 *  1. The call may fail in an undefined manner if either mapping is not
 *     tracked by <handle>.
 *  2. After executing a batch of unmaps, it is guaranteed that no stale
 *     mappings will remain in the device or host TLBs.
 */
#define GNTTABOP_unmap_and_replace    7
struct gnttab_unmap_and_replace {
    /* IN parameters. */
    uint64_t host_addr;
    uint64_t new_addr;
    grant_handle_t handle;
    /* OUT parameters. */
    int16_t  status;              /* => enum grant_status */
};

/*
 * GNTTABOP_set_version: Request a particular version of the grant
 * table shared table structure.  This operation may be used to toggle
 * between different versions, but must be performed while no grants
 * are active.  The only defined versions are 1 and 2.
 */
#define GNTTABOP_set_version          8
struct gnttab_set_version {
    /* IN/OUT parameters */
    uint32_t version;
};

/*
 * GNTTABOP_get_status_frames: Get the list of frames used to store grant
 * status for <dom>. In grant format version 2, the status is separated
 * from the other shared grant fields to allow more efficient synchronization
 * using barriers instead of atomic cmpxchg operations.
 */
#define GNTTABOP_get_status_frames    9
struct gnttab_get_status_frames {
    /* IN parameters. */
    uint32_t nr_frames;
    domid_t  dom;
    /* OUT parameters. */
    int16_t  status;              /* => enum grant_status */
    uint64_t *frame_list;
};

/*
 * Issue one or more cache maintenance operations on a portion of a
 * page granted to the calling domain by a foreign domain.
 */
#define GNTTABOP_cache_flush	      12
struct gnttab_cache_flush {
    union {
        uint64_t dev_bus_addr;
        grant_ref_t ref;
    } a;
    uint16_t offset; /* offset from start of grant */
    uint16_t length; /* size within the grant */
#define GNTTAB_CACHE_CLEAN          (1<<0)
#define GNTTAB_CACHE_INVAL          (1<<1)
#define GNTTAB_CACHE_SOURCE_GREF    (1<<31)
    uint32_t op;
};

#endif /* XEN_PUBLIC_GRANT_TABLE_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
