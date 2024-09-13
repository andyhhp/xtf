#include <arch/idt.h>
#include <arch/page.h>
#include <arch/processor.h>
#include <arch/segment.h>
#include <xtf/asm_macros.h>

/*

Stack frame layout:

|               Xen |          Hardware | Notes                |
|-------------------+-------------------+----------------------|
|               <r> |               <r> | <l>                  |
|-------------------+-------------------+----------------------|
|               %ss |               %ss | only on stack switch |
|              %esp |              %esp | only on stack switch |
|            eflags |            eflags |                      |
| upcall_mask / %cs |               %cs |                      |
|              %eip |              %eip |                      |
| %esp-> error_code | %esp-> error_code | if applicable        |

These stubs push an error_code of zero (if applicable) to make a common layout
for the frame.  A further word of metadata is then pushed, currently just
containing the entry vector.

*/

.macro env_IRET                 /* Environment specific version of `iret`. */
#if defined(CONFIG_PV)

        jmp HYPERCALL_iret      /* PV guests use the 'iret' hypercall. */

#else
        iretl                   /* HVM guests use a real 'iret' instruction. */
#endif
.endm

.macro exception_entry sym vec

ENTRY(entry_\sym)

        .if !((1 << \vec) & X86_EXC_HAVE_EC)
        /* Push dummy error code (if needed) to align stack. */
        push  $0
        .endif

        /* Push metadata (entry vector). */
        push  $\vec

        jmp   handle_exception

ENDFUNC(entry_\sym)
.endm

exception_entry DE  X86_EXC_DE
exception_entry DB  X86_EXC_DB
exception_entry NMI X86_EXC_NMI
exception_entry BP  X86_EXC_BP
exception_entry OF  X86_EXC_OF
exception_entry BR  X86_EXC_BR
exception_entry UD  X86_EXC_UD
exception_entry NM  X86_EXC_NM
exception_entry DF  X86_EXC_DF
exception_entry TS  X86_EXC_TS
exception_entry NP  X86_EXC_NP
exception_entry SS  X86_EXC_SS
exception_entry GP  X86_EXC_GP
exception_entry PF  X86_EXC_PF
exception_entry MF  X86_EXC_MF
exception_entry AC  X86_EXC_AC
exception_entry MC  X86_EXC_MC
exception_entry XM  X86_EXC_XM
exception_entry VE  X86_EXC_VE

        .align 16
handle_exception:

        push %es
        push %ds

        SAVE_ALL

        mov $__KERN_DS, %eax    /* Restore data segments. */
        mov %eax, %ds
        mov %eax, %es

        mov %esp, %eax          /* struct cpu_regs * */
        call do_exception

        RESTORE_ALL

        pop %ds
        pop %es

        add $8, %esp            /* Pop error_code/entry_vector. */

        env_IRET
ENDFUNC(handle_exception)


ENTRY(entry_ret_to_kernel)      /* int $X86_VEC_RET2KERN */
        mov %ebp, %esp          /* Restore %esp to exec_user_param()'s context. */
        ret
ENDFUNC(entry_ret_to_kernel)

ENTRY(exec_user_param)          /* %eax = ulong (*fn)(ulong p1), %edx = ulong p1 */
        push %ebp

        /* Prepare to "call" exec_user_stub(). */
        push $1f                /* Fake return addr as if we'd called exec_user_stub(). */
        mov %esp, %ebp          /* Stash %esp for entry_ret_to_kernel(). */

        /* Prepare an IRET frame. */
        push exec_user_ss       /* SS */
                                /* ESP */
        push $user_stack + PAGE_SIZE
        pushf                   /* EFLAGS */

        /* Apply and/or masks to eflags. */
        mov exec_user_efl_and_mask, %ecx
        and %ecx, (%esp)
        mov exec_user_efl_or_mask, %ecx
        or %ecx, (%esp)

        push exec_user_cs       /* CS */
        push $exec_user_stub    /* EIP */

        env_IRET                /* Drop to user privilege. */

1:      /* entry_ret_to_kernel() returns here with a sensible stack. */
        pop %ebp
        ret

ENDFUNC(exec_user_param)

.pushsection .text.user, "ax", @progbits
ENTRY(exec_user_stub)           /* %eax = ulong (*fn)(ulong p1), %edx = ulong p1 */
        xchg %eax, %edx         /* Swap p1 to be first parameter to fn(). */
        call *%edx              /* fn(p1) */

        int $X86_VEC_RET2KERN   /* Return to kernel privilege. */
ENDFUNC(exec_user_stub)
.popsection

ENTRY(entry_EVTCHN)
        push $0
        push $0x200

        push %es
        push %ds

        SAVE_ALL

        mov $__KERN_DS, %eax    /* Restore data segments. */
        mov %eax, %ds
        mov %eax, %es

        mov %esp, %eax          /* struct cpu_regs * */
        call do_evtchn

        RESTORE_ALL

        pop %ds
        pop %es

        add $8, %esp            /* Pop error_code/entry_vector. */

        env_IRET
ENDFUNC(entry_EVTCHN)

#if defined(CONFIG_PV)
ENTRY(entry_SYSCALL)
        push $0
        push $0x100

        push %es
        push %ds

        SAVE_ALL

        mov $__KERN_DS, %eax    /* Restore data segments. */
        mov %eax, %ds
        mov %eax, %es

        mov %esp, %eax          /* struct cpu_regs * */
        call do_syscall

        RESTORE_ALL

        pop %ds
        pop %es

        add $8, %esp            /* Pop error_code/entry_vector. */

        jmp HYPERCALL_iret
ENDFUNC(entry_SYSCALL)

ENTRY(entry_SYSENTER)
        push $0
        push $0x200

        push %es
        push %ds

        SAVE_ALL

        mov $__KERN_DS, %eax    /* Restore data segments. */
        mov %eax, %ds
        mov %eax, %es

        mov %esp, %eax          /* struct cpu_regs * */
        call do_sysenter

        RESTORE_ALL

        pop %ds
        pop %es

        add $8, %esp            /* Pop error_code/entry_vector. */

        jmp HYPERCALL_iret
ENDFUNC(entry_SYSENTER)
#endif

/*
 * Local variables:
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 */
