#include <arch/x86/page.h>
#include <xtf/asm_macros.h>
#include <xen/xen.h>

.text
.align PAGE_SIZE
GLOBAL(hypercall_page)
         /* Poisoned with `ret` for safety before hypercalls are set up. */
        .fill PAGE_SIZE, 1, 0xc3
        .size hypercall_page, PAGE_SIZE

DECLARE_HYPERCALL(console_io)
DECLARE_HYPERCALL(sched_op)
