#ifndef XTF_HYPERCALL_H
#define XTF_HYPERCALL_H

/*
 * Each architecture needs to define its own hypercall handling interface.
 * The hypercall handler should be named as follows:
 * hypercall_<hypercall_name>
 * e.g. hypercall_console_io
 * The reason for that is to have a standard way of calling hypercalls
 * in the common code.
 */
#include <arch/hypercall.h>

/* All Xen ABI for includers convenience .*/
#include <xen/callback.h>
#include <xen/elfnote.h>
#include <xen/errno.h>
#include <xen/event_channel.h>
#include <xen/grant_table.h>
#include <xen/hvm/hvm_op.h>
#include <xen/hvm/hvm_vcpu.h>
#include <xen/hvm/params.h>
#include <xen/memory.h>
#include <xen/physdev.h>
#include <xen/sched.h>
#include <xen/sysctl.h>
#include <xen/vcpu.h>
#include <xen/version.h>
#include <xen/xen.h>

/* Common hypercall helpers */
static inline long hypercall_shutdown(unsigned int reason)
{
    return hypercall_sched_op(SCHEDOP_shutdown, &reason);
}

static inline void hypercall_yield(void)
{
    hypercall_sched_op(SCHEDOP_yield, NULL);
}

static inline long hypercall_poll(evtchn_port_t port)
{
    struct sched_poll poll = { .ports = &port, .nr_ports = 1 };

    return hypercall_sched_op(SCHEDOP_poll, &poll);
}

static inline int hypercall_evtchn_send(evtchn_port_t port)
{
    return hypercall_event_channel_op(EVTCHNOP_send, &port);
}

static inline int hvm_set_param(unsigned int idx, uint64_t value)
{
    xen_hvm_param_t p = { .domid = DOMID_SELF, .index = idx, .value = value };

    return hypercall_hvm_op(HVMOP_set_param, &p);
}

static inline int hvm_get_param(unsigned int idx, uint64_t *value)
{
    xen_hvm_param_t p = { .domid = DOMID_SELF, .index = idx };
    int rc = hypercall_hvm_op(HVMOP_get_param, &p);

    if ( rc == 0 )
        *value = p.value;
    return rc;
}

#endif /* XTF_HYPERCALL_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
