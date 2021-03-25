#ifndef XTF_X86_HYPERCALL_H
#define XTF_X86_HYPERCALL_H

#include <arch/desc.h>
#include <arch/page.h>
#include <xen/sysctl.h>
#include <xen/callback.h>

#if defined(__x86_64__)

# include <arch/hypercall-x86_64.h>
# define HYPERCALL1 _hypercall64_1
# define HYPERCALL2 _hypercall64_2
# define HYPERCALL3 _hypercall64_3
# define HYPERCALL4 _hypercall64_4
# define HYPERCALL5 _hypercall64_5

#elif defined(__i386__)

# include <arch/hypercall-x86_32.h>
# define HYPERCALL1 _hypercall32_1
# define HYPERCALL2 _hypercall32_2
# define HYPERCALL3 _hypercall32_3
# define HYPERCALL4 _hypercall32_4
# define HYPERCALL5 _hypercall32_5

#endif

extern uint8_t hypercall_page[PAGE_SIZE];

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

static inline long hypercall_set_gdt(const unsigned long *mfns,
                                     unsigned int entries)
{
    return HYPERCALL2(long, __HYPERVISOR_set_gdt, mfns, entries);
}

static inline long hypercall_stack_switch(const unsigned int ss, const void *sp)
{
    return HYPERCALL2(long, __HYPERVISOR_stack_switch, ss, sp);
}

static inline long hypercall_update_descriptor(uint64_t maddr, user_desc desc)
{
#ifdef __x86_64__
    return HYPERCALL2(long, __HYPERVISOR_update_descriptor, maddr, desc.raw);
#else
    return HYPERCALL4(long, __HYPERVISOR_update_descriptor,
                      maddr, maddr >> 32, desc.lo, desc.hi);
#endif
}

/*
 * This hypercall is misnamed in the Xen ABI, and actually operates on a
 * linear address, not a virtual address.
 */
static inline long hypercall_update_va_mapping(
    unsigned long linear, uint64_t npte, enum XEN_UVMF flags)
{
#ifdef __x86_64__
    return HYPERCALL3(long, __HYPERVISOR_update_va_mapping, linear, npte, flags);
#else
    return HYPERCALL4(long, __HYPERVISOR_update_va_mapping,
                      linear, npte, npte >> 32, flags);
#endif
}

static inline long hypercall_mmuext_op(const mmuext_op_t ops[],
                                       unsigned int count,
                                       unsigned int *done,
                                       unsigned int foreigndom)
{
    return HYPERCALL4(long, __HYPERVISOR_mmuext_op,
                      ops, count, done, foreigndom);
}

static inline long hypercall_callback_op(unsigned int cmd, const void *arg)
{
    return HYPERCALL2(long, __HYPERVISOR_callback_op, cmd, arg);
}

static inline long hypercall_memory_op(unsigned int cmd, void *arg)
{
    return HYPERCALL2(long, __HYPERVISOR_memory_op, cmd, arg);
}

static inline long hypercall_multicall(struct multicall_entry *list,
                                       unsigned int nr)
{
    return HYPERCALL2(long, __HYPERVISOR_multicall, list, nr);
}

static inline long hypercall_xen_version(unsigned int cmd, void *arg)
{
    return HYPERCALL2(long, __HYPERVISOR_xen_version, cmd, arg);
}

static inline long hypercall_grant_table_op(unsigned int cmd, void *args,
                                            unsigned int count)
{
    return HYPERCALL3(long, __HYPERVISOR_grant_table_op, cmd, args, count);
}

static inline long hypercall_vm_assist(unsigned int cmd, unsigned int type)
{
    return HYPERCALL2(long, __HYPERVISOR_vm_assist, cmd, type);
}

static inline long hypercall_vcpu_op(unsigned int cmd, unsigned int vcpu,
                                     void *extra)
{
    return HYPERCALL3(long, __HYPERVISOR_vcpu_op, cmd, vcpu, extra);
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

static inline long hypercall_sysctl(xen_sysctl_t *arg)
{
    return HYPERCALL1(long, __HYPERVISOR_sysctl, arg);
}

static inline long hypercall_argo_op(unsigned int cmd, void *arg1, void *arg2,
                                     unsigned long arg3, unsigned long arg4)
{
    return HYPERCALL5(long, __HYPERVISOR_argo_op, cmd, arg1, arg2, arg3, arg4);
}

/*
 * Higher level hypercall helpers
 */
static inline int hypercall_register_callback(const xen_callback_register_t *arg)
{
    return hypercall_callback_op(CALLBACKOP_register, arg);
}

static inline void hypercall_console_write(const char *buf, unsigned long count)
{
    (void)HYPERCALL3(long, __HYPERVISOR_console_io, CONSOLEIO_write, count, buf);
}

#endif /* XTF_X86_HYPERCALL_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
