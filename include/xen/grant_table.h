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
