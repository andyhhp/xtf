#ifndef XEN_PUBLIC_EVENT_CHANNEL_H
#define XEN_PUBLIC_EVENT_CHANNEL_H

#include <xen/xen.h>

#define EVTCHNOP_send             4
#define EVTCHNOP_status           5
#define EVTCHNOP_alloc_unbound    6
#define EVTCHNOP_init_control    11
#define EVTCHNOP_expand_array    12

typedef uint32_t evtchn_port_t;

struct evtchn_status {
    /* IN parameters */
    domid_t dom;
    uint16_t _pad0;
    evtchn_port_t port;
    /* OUT parameters */
#define EVTCHN_STATUS_closed        0 /* Not in use */
#define EVTCHN_STATUS_unbound       1 /* Waiting for connection */
#define EVTCHN_STATUS_interdomain   2 /* Connected to remote domain */
#define EVTCHN_STATUS_pirq          3 /* Connected to a physical IRQ */
#define EVTCHN_STATUS_virq          4 /* Connected to a virtual IRQ */
#define EVTCHN_STATUS_ipi           5 /* Connected to a virtual IPI */
    uint32_t status;
    uint32_t vcpu;
    union {
        struct {
            domid_t dom;
        } unbound;                    /* EVTCHN_STATUS_unbound */
        struct {
            domid_t dom;
            evtchn_port_t port;
        } interdomain;                /* EVTCHN_STATUS_interdomain */
        uint32_t pirq;                /* EVTCHN_STATUS_pirq        */
        uint32_t virq;                /* EVTCHN_STATUS_virq        */
    };
};

struct evtchn_alloc_unbound {
    /* IN parameters. */
    domid_t dom, remote_dom;
    /* OUT parameters. */
    evtchn_port_t port;
};

struct evtchn_init_control {
    /* IN parameters. */
    uint64_t control_gfn;
    uint32_t offset;
    uint32_t vcpu;
    /* OUT parameters. */
    uint8_t link_bits;
    uint8_t _pad[7];
};

struct evtchn_expand_array {
    /* IN parameters. */
    uint64_t array_gfn;
};

#endif /* XEN_PUBLIC_EVENT_CHANNEL_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
