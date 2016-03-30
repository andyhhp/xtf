#ifndef XEN_PUBLIC_IO_XS_WIRE_H
#define XEN_PUBLIC_IO_XS_WIRE_H

/* API/ABI relevent to the XenBus shared memory interface. */

#define XENBUS_RING_SIZE 1024
static inline uint32_t mask_xenbus_idx(uint32_t idx)
{
    return idx & (XENBUS_RING_SIZE - 1);
}

struct xenbus_interface {
    char req[XENBUS_RING_SIZE]; /* Requests to the xenstore daemon. */
    char rsp[XENBUS_RING_SIZE]; /* Replies and async watch events. */
    uint32_t req_cons, req_prod;
    uint32_t rsp_cons, rsp_prod;
    uint32_t server_features;
    uint32_t connection;
};
typedef struct xenbus_interface xenbus_interface_t;

#define XENBUS_SERVER_FEATURE_RECONNECTION 1

#define XENBUS_CONNECTED 0
#define XENBUS_RECONNECT 1

#endif /* XEN_PUBLIC_IO_XS_WIRE_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
