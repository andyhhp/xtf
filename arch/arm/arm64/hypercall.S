#include <xtf/asm_macros.h>
#include <xen/xen.h>

#define HYPERCALL_SIMPLE(hypercall)     \
ENTRY(hypercall_##hypercall)            \
    mov x16, #__HYPERVISOR_##hypercall; \
    hvc XEN_HYPERCALL_TAG;              \
    ret;                                \
ENDFUNC(hypercall_##hypercall)

#define HYPERCALL0 HYPERCALL_SIMPLE
#define HYPERCALL1 HYPERCALL_SIMPLE
#define HYPERCALL2 HYPERCALL_SIMPLE
#define HYPERCALL3 HYPERCALL_SIMPLE
#define HYPERCALL4 HYPERCALL_SIMPLE
#define HYPERCALL5 HYPERCALL_SIMPLE

.text

HYPERCALL2(xen_version);
HYPERCALL3(console_io);
HYPERCALL3(grant_table_op);
HYPERCALL2(sched_op);
HYPERCALL2(event_channel_op);
HYPERCALL2(hvm_op);
HYPERCALL2(memory_op);
HYPERCALL2(physdev_op);
HYPERCALL3(vcpu_op);
HYPERCALL1(tmem_op);
HYPERCALL2(multicall);
HYPERCALL2(vm_assist);
HYPERCALL1(sysctl);
HYPERCALL1(domctl);
