#include <xtf/asm_macros.h>
#include <xen/xen.h>

#define __HVC(imm16) .long \
    ((0xE1400070 | (((imm16) & 0xFFF0) << 4) | ((imm16) & 0x000F)) & 0xFFFFFFFF)

#define HYPERCALL_SIMPLE(hypercall)     \
ENTRY(hypercall_##hypercall)            \
    mov r12, #__HYPERVISOR_##hypercall; \
    __HVC(XEN_HYPERCALL_TAG);           \
    mov pc ,lr;                         \
ENDFUNC(hypercall_##hypercall)

#define HYPERCALL0 HYPERCALL_SIMPLE
#define HYPERCALL1 HYPERCALL_SIMPLE
#define HYPERCALL2 HYPERCALL_SIMPLE
#define HYPERCALL3 HYPERCALL_SIMPLE
#define HYPERCALL4 HYPERCALL_SIMPLE

#define HYPERCALL5(hypercall)           \
ENTRY(hypercall_##hypercall)            \
    stmdb sp!, {r4};                    \
    ldr r4, [sp, #4];                   \
    mov r12, #__HYPERVISOR_##hypercall; \
    __HVC(XEN_HYPERCALL_TAG);           \
    ldm sp!, {r4};                      \
    mov pc ,lr;                         \
ENDFUNC(hypercall_##hypercall)

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
