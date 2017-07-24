#include <xtf/types.h>
#include <xtf/atomic.h>
#include <xtf/bitops.h>
#include <xtf/console.h>
#include <xtf/hypercall.h>
#include <xtf/lib.h>
#include <xtf/libc.h>
#include <xtf/traps.h>

/*
 * Output functions, registered if/when available.
 * Possibilities:
 * - Xen hypervisor console
 * - PV console
 * - Qemu debug console
 */
static cons_output_cb output_fns[3];
static unsigned int nr_cons_cb;

/* Guest PV console details. */
static xencons_interface_t *pv_ring;
static evtchn_port_t pv_evtchn;

void register_console_callback(cons_output_cb fn)
{
    if ( nr_cons_cb < ARRAY_SIZE(output_fns) )
        output_fns[nr_cons_cb++] = fn;
    else
        panic("Too many console callbacks\n");
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
 * Read out data from the pv ring, either until buffer is filled or no
 * more data are available. Might result in partial strings, depending
 * on how xenconsoled passes in data.
 *
 * Will block if no data are available.
 */
size_t pv_console_read_some(char *buf, size_t len)
{
    size_t s = 0;
    uint32_t cons, prod;

    while ( !test_and_clear_bit(pv_evtchn, shared_info.evtchn_pending) ||
            (pv_ring->in_cons == pv_ring->in_prod) )
        hypercall_poll(pv_evtchn);

    cons = pv_ring->in_cons;
    prod = LOAD_ACQUIRE(&pv_ring->in_prod);

    while ( (s < len) && (0 < (prod - cons)) )
        buf[s++] = pv_ring->in[cons++ & (sizeof(pv_ring->in) - 1)];

    STORE_RELEASE(&pv_ring->in_cons, cons);

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
    if ( port >= (sizeof(shared_info.evtchn_pending) * CHAR_BIT) )
        panic("evtchn %u out of evtchn_pending[] range\n", port);

    pv_ring = ring;
    pv_evtchn = port;
    register_console_callback(pv_console_write);
}

void vprintk(const char *fmt, va_list args)
{
    static char buf[2048];
    unsigned int i;
    int rc;

    rc = vsnprintf(buf, sizeof(buf), fmt, args);

    if ( rc > (int)sizeof(buf) )
        panic("vprintk() buffer overflow\n");

    for ( i = 0; i < nr_cons_cb; ++i )
        output_fns[i](buf, rc);
}

void printk(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);
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
