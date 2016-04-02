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


/* API/ABI relevent to the Xenstore protocol. */

struct xenstore_msg_hdr {
    uint32_t type;
    uint32_t req_id;
    uint32_t tx_id;
    uint32_t len;
};

enum xenstore_msg_type
{
    XS_CONTROL,
    XS_DIRECTORY,
    XS_READ,
    XS_GET_PERMS,
    XS_WATCH,
    XS_UNWATCH,
    XS_TRANSACTION_START,
    XS_TRANSACTION_END,
    XS_INTRODUCE,
    XS_RELEASE,
    XS_GET_DOMAIN_PATH,
    XS_WRITE,
    XS_MKDIR,
    XS_RM,
    XS_SET_PERMS,
    XS_WATCH_EVENT,
    XS_ERROR,
    XS_IS_DOMAIN_INTRODUCED,
    XS_RESUME,
    XS_SET_TARGET,
    /* XS_RESTRICT has been removed */
    XS_RESET_WATCHES = XS_SET_TARGET + 2,
    XS_DIRECTORY_PART,

    XS_TYPE_COUNT,      /* Number of valid types. */

    XS_INVALID = 0xffff /* Guaranteed to remain an invalid type */
};


#define XENSTORE_PAYLOAD_MAX 4096

#define XENSTORE_ABS_PATH_MAX 3072
#define XENSTORE_REL_PATH_MAX 2048

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
