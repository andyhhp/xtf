#include <arch/idt.h>
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
        iret                    /* HVM guests use a real 'iret' instruction. */
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

        SAVE_ALL

#if defined(CONFIG_HVM)
        testl $X86_EFLAGS_VM, 11*4(%esp)
        jz 1f

        mov $__KERN_DS, %eax    /* Restore data segments if leaving vm86 mode. */
        mov %eax, %ds
        mov %eax, %es
        mov %eax, %fs
        mov %eax, %gs
1:
#endif

        push %esp               /* struct cpu_regs * */
        call do_exception
        add $4, %esp

        RESTORE_ALL
        add $8, %esp            /* Pop error_code/entry_vector. */

        env_IRET
ENDFUNC(handle_exception)


ENTRY(entry_ret_to_kernel)      /* int $X86_VEC_RET2KERN */

        /* User required to ensure this is called from CPL > KERNEL_RPL */

        mov 0*4(%esp), %ecx     /* Stash %eip from iret frame */
        mov 3*4(%esp), %esp     /* Load %esp from iret frame  */
        jmp *%ecx               /* Jump back                  */

ENDFUNC(entry_ret_to_kernel)

ENTRY(exec_user_param)
        /*
         * 2*4(%esp) ulong p1
         * 1*4(%esp) ulong (*fn)(ulong)
         * 0*4(%esp) return address
         */

        push $__USER_DS         /* SS */
        push %esp
        addl $4, (%esp)         /* ESP */
        pushf                   /* EFLAGS */

#if defined(CONFIG_PV)          /* PV guests see the real interrupt flag. Clobber it. */
        andl $~X86_EFLAGS_IF, (%esp)
#endif

        push $__USER_CS         /* CS */
        push $1f                /* EIP */

        env_IRET                /* Drop to user privilege. */
1:
        push 2*4(%esp)          /* Re-push p1 for fn()'s call frame. */
        call *(1+1)*4(%esp)     /* fn(p1) */
        add $1*4, %esp          /* Pop p1. */

        int $X86_VEC_RET2KERN   /* Return to kernel privilege. */
        ret

ENDFUNC(exec_user_param)

ENTRY(exec_user)
        /*
         * 1*4(%esp) ulong (*fn)(ulong)
         * 0*4(%esp) return address
         */
        push $0xdead0000        /* Poison unused p1. */
        push (1+1)*4(%esp)      /* Re-push fn for exec_user_param()'s call frame. */
        call exec_user_param
        add $2*4, %esp          /* Pop fn/p1. */
        ret
ENDFUNC(exec_user)

/*
 * Local variables:
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 */
