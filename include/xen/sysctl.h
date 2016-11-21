/******************************************************************************
 * sysctl.h
 *
 * System management operations. For use by node control stack.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Copyright (c) 2002-2006, K Fraser
 */

#ifndef __XEN_PUBLIC_SYSCTL_H__
#define __XEN_PUBLIC_SYSCTL_H__

#include "xen.h"
#include "physdev.h"

/*
 * XEN_SYSCTL_LIVEPATCH_op
 *
 * Refer to the docs/unstable/misc/livepatch.markdown
 * for the design details of this hypercall.
 *
 * There are four sub-ops:
 *  XEN_SYSCTL_LIVEPATCH_UPLOAD (0)
 *  XEN_SYSCTL_LIVEPATCH_GET (1)
 *  XEN_SYSCTL_LIVEPATCH_LIST (2)
 *  XEN_SYSCTL_LIVEPATCH_ACTION (3)
 *
 * The normal sequence of sub-ops is to:
 *  1) XEN_SYSCTL_LIVEPATCH_UPLOAD to upload the payload. If errors STOP.
 *  2) XEN_SYSCTL_LIVEPATCH_GET to check the `->rc`. If -XEN_EAGAIN spin.
 *     If zero go to next step.
 *  3) XEN_SYSCTL_LIVEPATCH_ACTION with LIVEPATCH_ACTION_APPLY to apply the patch.
 *  4) XEN_SYSCTL_LIVEPATCH_GET to check the `->rc`. If in -XEN_EAGAIN spin.
 *     If zero exit with success.
 */

#define LIVEPATCH_PAYLOAD_VERSION 1
/*
 * Structure describing an ELF payload. Uniquely identifies the
 * payload. Should be human readable.
 * Recommended length is upto XEN_LIVEPATCH_NAME_SIZE.
 * Includes the NUL terminator.
 */
#define XEN_LIVEPATCH_NAME_SIZE 128
struct xen_livepatch_name {
    guest_handle_64_t name;                 /* IN: pointer to name. */
    uint16_t size;                          /* IN: size of name. May be upto
                                               XEN_LIVEPATCH_NAME_SIZE. */
    uint16_t pad[3];                        /* IN: MUST be zero. */
};
typedef struct xen_livepatch_name xen_livepatch_name_t;

/*
 * Upload a payload to the hypervisor. The payload is verified
 * against basic checks and if there are any issues the proper return code
 * will be returned. The payload is not applied at this time - that is
 * controlled by XEN_SYSCTL_LIVEPATCH_ACTION.
 *
 * The return value is zero if the payload was succesfully uploaded.
 * Otherwise an EXX return value is provided. Duplicate `name` are not
 * supported.
 *
 * The payload at this point is verified against basic checks.
 *
 * The `payload` is the ELF payload as mentioned in the `Payload format`
 * section in the Live Patch design document.
 */
#define XEN_SYSCTL_LIVEPATCH_UPLOAD 0
struct xen_sysctl_livepatch_upload {
    xen_livepatch_name_t name;              /* IN, name of the patch. */
    uint64_t size;                          /* IN, size of the ELF file. */
    guest_handle_64_t payload;              /* IN, the ELF file. */
};
typedef struct xen_sysctl_livepatch_upload xen_sysctl_livepatch_upload_t;

/*
 * Retrieve an status of an specific payload.
 *
 * Upon completion the `struct xen_livepatch_status` is updated.
 *
 * The return value is zero on success and XEN_EXX on failure. This operation
 * is synchronous and does not require preemption.
 */
#define XEN_SYSCTL_LIVEPATCH_GET 1

struct xen_livepatch_status {
#define LIVEPATCH_STATE_CHECKED      1
#define LIVEPATCH_STATE_APPLIED      2
    uint32_t state;                /* OUT: LIVEPATCH_STATE_*. */
    int32_t rc;                    /* OUT: 0 if no error, otherwise -XEN_EXX. */
};
typedef struct xen_livepatch_status xen_livepatch_status_t;

struct xen_sysctl_livepatch_get {
    xen_livepatch_name_t name;              /* IN, name of the payload. */
    xen_livepatch_status_t status;          /* IN/OUT, state of it. */
};
typedef struct xen_sysctl_livepatch_get xen_sysctl_livepatch_get_t;

/*
 * Retrieve an array of abbreviated status and names of payloads that are
 * loaded in the hypervisor.
 *
 * If the hypercall returns an positive number, it is the number (up to `nr`)
 * of the payloads returned, along with `nr` updated with the number of remaining
 * payloads, `version` updated (it may be the same across hypercalls. If it
 * varies the data is stale and further calls could fail). The `status`,
 * `name`, and `len`' are updated at their designed index value (`idx`) with
 * the returned value of data.
 *
 * If the hypercall returns E2BIG the `nr` is too big and should be
 * lowered. The upper limit of `nr` is left to the implemention.
 *
 * Note that due to the asynchronous nature of hypercalls the domain might have
 * added or removed the number of payloads making this information stale. It is
 * the responsibility of the toolstack to use the `version` field to check
 * between each invocation. if the version differs it should discard the stale
 * data and start from scratch. It is OK for the toolstack to use the new
 * `version` field.
 */
#define XEN_SYSCTL_LIVEPATCH_LIST 2
struct xen_sysctl_livepatch_list {
    uint32_t version;                       /* OUT: Hypervisor stamps value.
                                               If varies between calls, we are
                                             * getting stale data. */
    uint32_t idx;                           /* IN: Index into hypervisor list. */
    uint32_t nr;                            /* IN: How many status, name, and len
                                               should fill out. Can be zero to get
                                               amount of payloads and version.
                                               OUT: How many payloads left. */
    uint32_t pad;                           /* IN: Must be zero. */
    guest_handle_64_t status;               /* OUT. Must have enough
                                               space allocate for nr of them. */
    guest_handle_64_t name;                 /* OUT: Array of names. Each member
                                               MUST XEN_LIVEPATCH_NAME_SIZE in size.
                                               Must have nr of them. */
    guest_handle_64_t len;                  /* OUT: Array of lengths of name's.
                                               Must have nr of them. */
};
typedef struct xen_sysctl_livepatch_list xen_sysctl_livepatch_list_t;

/*
 * Perform an operation on the payload structure referenced by the `name` field.
 * The operation request is asynchronous and the status should be retrieved
 * by using either XEN_SYSCTL_LIVEPATCH_GET or XEN_SYSCTL_LIVEPATCH_LIST hypercall.
 */
#define XEN_SYSCTL_LIVEPATCH_ACTION 3
struct xen_sysctl_livepatch_action {
    xen_livepatch_name_t name;              /* IN, name of the patch. */
#define LIVEPATCH_ACTION_UNLOAD       1
#define LIVEPATCH_ACTION_REVERT       2
#define LIVEPATCH_ACTION_APPLY        3
#define LIVEPATCH_ACTION_REPLACE      4
    uint32_t cmd;                           /* IN: LIVEPATCH_ACTION_*. */
    uint32_t timeout;                       /* IN: Zero if no timeout. */
                                            /* Or upper bound of time (ms) */
                                            /* for operation to take. */
};
typedef struct xen_sysctl_livepatch_action xen_sysctl_livepatch_action_t;

struct xen_sysctl_livepatch_op {
    uint32_t cmd;                           /* IN: XEN_SYSCTL_LIVEPATCH_*. */
    uint32_t pad;                           /* IN: Always zero. */
    union {
        xen_sysctl_livepatch_upload_t upload;
        xen_sysctl_livepatch_list_t list;
        xen_sysctl_livepatch_get_t get;
        xen_sysctl_livepatch_action_t action;
    } u;
};
typedef struct xen_sysctl_livepatch_op xen_sysctl_livepatch_op_t;

struct xen_sysctl {
    uint32_t cmd;
#define XEN_SYSCTL_livepatch_op                  27
    uint32_t interface_version; /* XEN_SYSCTL_INTERFACE_VERSION */
    union {
        struct xen_sysctl_livepatch_op      livepatch;
        uint8_t                             pad[128];
    } u;
};
typedef struct xen_sysctl xen_sysctl_t;

#endif /* __XEN_PUBLIC_SYSCTL_H__ */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
