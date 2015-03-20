#include <xtf/types.h>
#include <xtf/atomic.h>
#include <xtf/console.h>
#include <xtf/hypercall.h>
#include <xtf/lib.h>
#include <xtf/libc.h>

/*
 * Output functions, registered if/when available.
 * Possibilities:
 * - Xen hypervisor console
 * - PV console
 */
static cons_output_cb output_fns[2];
static unsigned int nr_cons_cb;

/* Guest PV console details. */
static xencons_interface_t *pv_ring;
static evtchn_port_t pv_evtchn;

void register_console_callback(cons_output_cb fn)
{
    if ( nr_cons_cb < ARRAY_SIZE(output_fns) )
        output_fns[nr_cons_cb++] = fn;
}

/*
 * Write some data into the pv ring, taking care not to overflow the ring.
 */
static size_t pv_console_write_some(const char *buf, size_t len)
{
    size_t s = 0;
    uint32_t cons = LOAD_ACQUIRE(&pv_ring->out_cons), prod = pv_ring->out_prod;

    while ( (s < len) && ((prod - cons) < sizeof(pv_ring->out)) )
        pv_ring->out[prod++ & (sizeof(pv_ring->out) - 1)] = buf[s++];

    STORE_RELEASE(&pv_ring->out_prod, prod);

    return s;
}

/*
 * Write some data into the pv ring, synchronously waiting for all data to be
 * consumed.
 */
static void pv_console_write(const char *buf, size_t len)
{
    size_t written = 0;
    uint32_t cons = LOAD_ACQUIRE(&pv_ring->out_cons);

    do
    {
        /* Try and put some data into the ring. */
        written = pv_console_write_some(&buf[written], len - written);

        /* Kick xenconsoled into action. */
        hypercall_evtchn_send(pv_evtchn);

        /*
         * If we have more to write, the ring must have filled up.  Wait for
         * more space.
         */
        if ( written < len )
        {
            while ( ACCESS_ONCE(pv_ring->out_cons) == cons )
                hypercall_yield();
        }

    } while ( written < len );

    /* Wait for xenconsoled to consume all the data we gave. */
    while ( ACCESS_ONCE(pv_ring->out_cons) != pv_ring->out_prod )
        hypercall_yield();
}

void init_pv_console(xencons_interface_t *ring, evtchn_port_t port)
{
    pv_ring = ring;
    pv_evtchn = port;
    register_console_callback(pv_console_write);
}

void printk(const char *fmt, ...)
{
    unsigned int i;

    for ( i = 0; i < nr_cons_cb; ++i )
        output_fns[i](fmt, strlen(fmt));
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
