/**
 * @file arch/arm/include/arch/hypercall.h
 *
 * Hypercall primitives for arm.
 */
#ifndef XTF_ARM_HYPERCALL_H
#define XTF_ARM_HYPERCALL_H

#include <xtf/lib.h>
#include <arch/desc.h>
#include <arch/page.h>
#include <xen/sysctl.h>

int hypercall_memory_op(unsigned int cmd, void *arg);
int hypercall_domctl(unsigned long op);
int hypercall_sched_op(int cmd, void *arg);
int hypercall_console_io(int cmd, int count, char *str);
int hypercall_xen_version(int cmd, void *arg);
int hypercall_event_channel_op(int cmd, void *op);
int hypercall_physdev_op(void *physdev_op);
int hypercall_sysctl(xen_sysctl_t *arg);
int hypercall_hvm_op(unsigned long op, void *arg);
int hypercall_grant_table_op(unsigned int cmd, void *uop, unsigned int count);
int hypercall_vcpu_op(int cmd, int vcpuid, void *extra_args);

/*
 * Higher level hypercall helpers
 */
static inline void hypercall_console_write(const char *buf, size_t count)
{
    (void)hypercall_console_io(CONSOLEIO_write, count, (char *)buf);
}

#endif /* XTF_ARM_HYPERCALL_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
