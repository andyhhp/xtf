/*
 * Xen public memory_op hypercall interface
 */

#ifndef XEN_PUBLIC_MEMORY_H
#define XEN_PUBLIC_MEMORY_H

#define XENMEM_increase_reservation 0
#define XENMEM_decrease_reservation 1
#define XENMEM_populate_physmap     6

struct xen_memory_reservation {
    unsigned long *extent_start;
    unsigned long nr_extents;
    unsigned int extent_order;
    unsigned int mem_flags;
    domid_t domid;
};

#define XENMEM_add_to_physmap       7

struct xen_add_to_physmap {
    domid_t domid;
    uint16_t size;

#define XENMAPSPACE_shared_info  0
#define XENMAPSPACE_grant_table  1
#define XENMAPSPACE_gmfn         2
#define XENMAPSPACE_gmfn_range   3
#define XENMAPSPACE_gmfn_foreign 4
#define XENMAPSPACE_dev_mmio     5
    unsigned int space;

    unsigned long idx;
    unsigned long gfn;
};

#define XENMEM_exchange             11

struct xen_memory_exchange {
    struct xen_memory_reservation in;
    struct xen_memory_reservation out;
    unsigned long nr_exchanged;
};

#define XENMEM_maximum_gpfn         14

#endif /* XEN_PUBLIC_MEMORY_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
