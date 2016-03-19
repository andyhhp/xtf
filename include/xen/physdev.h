/*
 * Xen public physdev_op hypercall interface
 */

#ifndef XEN_PUBLIC_PHYSDEV_H
#define XEN_PUBLIC_PHYSDEV_H

/*
 * Set the current VCPU's I/O privilege level.
 * @arg == pointer to physdev_set_iopl structure.
 */
#define PHYSDEVOP_set_iopl               6
#ifndef __ASSEMBLY__
struct physdev_set_iopl {
    /* IN */
    uint32_t iopl;
};
#endif

#endif /* XEN_PUBLIC_PHYSDEV_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
