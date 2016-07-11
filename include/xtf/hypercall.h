#ifndef XTF_HYPERCALL_H
#define XTF_HYPERCALL_H

#include <xtf/types.h>
#include <arch/x86/page.h>

#if defined(__x86_64__)

# include <arch/x86/x86_64/hypercall-x86_64.h>
# define HYPERCALL1 _hypercall64_1
# define HYPERCALL2 _hypercall64_2
# define HYPERCALL3 _hypercall64_3
# define HYPERCALL4 _hypercall64_4

#elif defined(__i386__)

# include <arch/x86/x86_32/hypercall-x86_32.h>
# define HYPERCALL1 _hypercall32_1
# define HYPERCALL2 _hypercall32_2
# define HYPERCALL3 _hypercall32_3
# define HYPERCALL4 _hypercall32_4

#else
# error Bad architecture for hypercalls
#endif

extern uint8_t hypercall_page[PAGE_SIZE];

/* All Xen ABI for includers convenience .*/
#include <xen/xen.h>
#include <xen/sched.h>
#include <xen/errno.h>
#include <xen/event_channel.h>
#include <xen/physdev.h>
#include <xen/memory.h>
#include <xen/version.h>
#include <xen/hvm/hvm_op.h>
#include <xen/hvm/params.h>

/*
 * Hypercall primatives, compiled for the correct bitness
 */
static inline long hypercall_set_trap_table(const struct xen_trap_info *ti)
{
    return HYPERCALL1(long, __HYPERVISOR_set_trap_table, ti);
}

static inline long hypercall_mmu_update(const mmu_update_t reqs[],
                                        unsigned int count,
                                        unsigned int *done,
                                        unsigned int foreigndom)
{
    return HYPERCALL4(long, __HYPERVISOR_mmu_update,
                      reqs, count, done, foreigndom);
}

static inline long hypercall_stack_switch(const unsigned int ss, const void *sp)
{
    return HYPERCALL2(long, __HYPERVISOR_stack_switch, ss, sp);
}

static inline long hypercall_memory_op(unsigned int cmd, void *arg)
{
    return HYPERCALL2(long, __HYPERVISOR_memory_op, cmd, arg);
}

static inline long hypercall_xen_version(unsigned cmd, void *arg)
{
    return HYPERCALL2(long, __HYPERVISOR_xen_version, cmd, arg);
}

static inline long hypercall_update_va_mapping(void *va, uint64_t npte,
                                               unsigned int flags)
{
#ifdef __x86_64__
    return HYPERCALL3(long, __HYPERVISOR_update_va_mapping, va, npte, flags);
#else
    return HYPERCALL4(long, __HYPERVISOR_update_va_mapping,
                      va, npte, npte >> 32, flags);
#endif
}

static inline long hypercall_vm_assist(unsigned int cmd, unsigned int type)
{
    return HYPERCALL2(long, __HYPERVISOR_vm_assist, cmd, type);
}

static inline long hypercall_mmuext_op(const mmuext_op_t ops[],
                                       unsigned int count,
                                       unsigned int *done,
                                       unsigned int foreigndom)
{
    return HYPERCALL4(long, __HYPERVISOR_mmuext_op,
                      ops, count, done, foreigndom);
}

static inline long hypercall_sched_op(unsigned int cmd, void *arg)
{
    return HYPERCALL2(long, __HYPERVISOR_sched_op, cmd, arg);
}

static inline long hypercall_event_channel_op(unsigned int cmd, void *arg)
{
    return HYPERCALL2(long, __HYPERVISOR_event_channel_op, cmd, arg);
}

static inline long hypercall_physdev_op(unsigned int cmd, void *arg)
{
    return HYPERCALL2(long, __HYPERVISOR_physdev_op, cmd, arg);
}

static inline long hypercall_hvm_op(unsigned int cmd, void *arg)
{
    return HYPERCALL2(long, __HYPERVISOR_hvm_op, cmd, arg);
}

/*
 * Higher level hypercall helpers
 */
static inline void hypercall_console_write(const char *buf, unsigned long count)
{
    (void)HYPERCALL3(long, __HYPERVISOR_console_io, CONSOLEIO_write, count, buf);
}

static inline long hypercall_shutdown(unsigned int reason)
{
    return hypercall_sched_op(SCHEDOP_shutdown, &reason);
}

static inline void hypercall_yield(void)
{
    hypercall_sched_op(SCHEDOP_yield, NULL);
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
