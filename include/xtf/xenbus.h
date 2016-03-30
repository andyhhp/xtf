#ifndef XTF_XENBUS_H
#define XTF_XENBUS_H

#include <xtf/types.h>
#include <xen/event_channel.h>
#include <xen/io/xs_wire.h>

void init_xenbus(xenbus_interface_t *ring, evtchn_port_t port);

#endif /* XTF_XENBUS_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
