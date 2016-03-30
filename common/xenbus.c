#include <xtf/lib.h>
#include <xtf/traps.h>
#include <xtf/xenbus.h>

static xenbus_interface_t *xb_ring;
static evtchn_port_t xb_port;

void init_xenbus(xenbus_interface_t *ring, evtchn_port_t port)
{
    if ( port >= (sizeof(shared_info.evtchn_pending) * CHAR_BIT) )
        panic("evtchn %u out of evtchn_pending[] range\n", port);

    xb_ring = ring;
    xb_port = port;
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
