/*
 * Xen public xen_version hypercall interface
 */

#ifndef XEN_PUBLIC_SCHED_H
#define XEN_PUBLIC_SCHED_H

#include "event_channel.h"

#define SCHEDOP_yield    0
#define SCHEDOP_shutdown 2
#define SCHEDOP_poll     3

#ifndef __ASSEMBLY__
struct sched_shutdown {
    unsigned int reason; /* SHUTDOWN_* */
};

struct sched_poll {
    evtchn_port_t *ports;
    unsigned int nr_ports;
    uint64_t timeout;
};
#endif

#define SHUTDOWN_poweroff   0  /* Domain exited normally. Clean up and kill. */
#define SHUTDOWN_reboot     1  /* Clean up, kill, and then restart.          */
#define SHUTDOWN_suspend    2  /* Clean up, save suspend info, kill.         */
#define SHUTDOWN_crash      3  /* Tell controller we've crashed.             */
#define SHUTDOWN_watchdog   4  /* Restart because watchdog time expired.     */

#endif /* XEN_PUBLIC_SCHED_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
