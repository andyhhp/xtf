/*
 * Xen public domctl hypercall interface
 */

#ifndef XEN_PUBLIC_DOMCTL_H
#define XEN_PUBLIC_DOMCTL_H

struct xen_domctl_createdomain {
    /* IN parameters */
    uint32_t ssidref;
    uint8_t handle[16];
 /* Is this an HVM guest (as opposed to a PVH or PV guest)? */
#define _XEN_DOMCTL_CDF_hvm_guest     0
#define XEN_DOMCTL_CDF_hvm_guest      (1U<<_XEN_DOMCTL_CDF_hvm_guest)
 /* Use hardware-assisted paging if available? */
#define _XEN_DOMCTL_CDF_hap           1
#define XEN_DOMCTL_CDF_hap            (1U<<_XEN_DOMCTL_CDF_hap)
 /* Should domain memory integrity be verifed by tboot during Sx? */
#define _XEN_DOMCTL_CDF_s3_integrity  2
#define XEN_DOMCTL_CDF_s3_integrity   (1U<<_XEN_DOMCTL_CDF_s3_integrity)
 /* Disable out-of-sync shadow page tables? */
#define _XEN_DOMCTL_CDF_oos_off       3
#define XEN_DOMCTL_CDF_oos_off        (1U<<_XEN_DOMCTL_CDF_oos_off)
 /* Is this a xenstore domain? */
#define _XEN_DOMCTL_CDF_xs_domain     4
#define XEN_DOMCTL_CDF_xs_domain      (1U<<_XEN_DOMCTL_CDF_xs_domain)
    uint32_t flags;

    /*
     * Various domain limits, which impact the quantity of resources (global
     * mapping space, xenheap, etc) a guest may consume.
     */
    uint32_t max_vcpus;
    uint32_t max_evtchn_port;
    uint32_t max_grant_frames;
    uint32_t max_maptrack_frames;

    struct xen_arch_domainconfig arch;
};

struct xen_domctl {
    uint32_t cmd;
#define XEN_DOMCTL_createdomain                   1
#define XEN_DOMCTL_destroydomain                  2
    uint32_t interface_version; /* XEN_DOMCTL_INTERFACE_VERSION */
    domid_t  domain;
    union {
        struct xen_domctl_createdomain      createdomain;
        uint8_t                             pad[128];
    } u;
};
typedef struct xen_domctl xen_domctl_t;

#endif /* XEN_PUBLIC_DOMCTL_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
