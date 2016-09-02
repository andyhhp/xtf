#ifndef XEN_PUBLIC_EVENT_CHANNEL_H
#define XEN_PUBLIC_EVENT_CHANNEL_H

#define EVTCHNOP_send             4
#define EVTCHNOP_init_control    11
#define EVTCHNOP_expand_array    12

typedef uint32_t evtchn_port_t;

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
