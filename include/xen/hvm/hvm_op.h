/*
 * Xen public hvmop interface
 */

#ifndef XEN_PUBLIC_HVM_HVM_OP_H
#define XEN_PUBLIC_HVM_HVM_OP_H

/* Get/set subcommands: extra argument == pointer to xen_hvm_param struct. */
#define HVMOP_set_param           0
#define HVMOP_get_param           1
struct xen_hvm_param {
    domid_t  domid;
    uint32_t index;
    uint64_t value;
};
typedef struct xen_hvm_param xen_hvm_param_t;

#endif /* XEN_PUBLIC_HVM_HVM_OP_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
