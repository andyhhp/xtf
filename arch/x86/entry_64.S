#include <arch/idt.h>
#include <arch/page.h>
#include <arch/processor.h>
#include <arch/segment.h>
#include <xtf/asm_macros.h>

/*

Stack frame layout: (first aligned to 16 byte boundary)

|               Xen |          Hardware | Notes         |
|-------------------+-------------------+---------------|
|               <r> |               <r> | <l>           |
|-------------------+-------------------+---------------|
|               %ss |               %ss |               |
|              %rsp |              %rsp |               |
|            rflags |            rflags |               |
| upcall_mask / %cs |               %cs |               |
|              %rip |              %rip |               |
|        error_code | %rsp-> error_code | if applicable |
|              %r11 |                   |               |
| %rsp->       %rcx |                   |               |

The %rcx and %r11 parameters are because Xen will typically SYSRET to the
entry point; they should be restored promptly.

The stubs then push an error_code (if required) to make a common layout for
the frame, then use the upper 32bits of the error_code to stash additional
metadata.  Currently just the entry vector.

*/

.macro env_ADJUST_FRAME         /* Environment specific exception entry. */
#if defined(CONFIG_PV)
                                /* Restore results of Xen SYSRET'ing to this point. */
        pop   %rcx
        pop   %r11
#endif
.endm

.macro env_IRETQ                /* Environment specific version of `iretq`. */
#if defined(CONFIG_PV)

        push $0                 /* Indicate that this isn't a SYSRET'able   */
        jmp HYPERCALL_iret      /* situation, and use the 'iret' hypercall. */

#else
        iretq                   /* HVM guests use a real 'iretq' instruction. */
#endif
.endm

.macro exception_entry sym vec

ENTRY(entry_\sym)
        env_ADJUST_FRAME

        .if !((1 << \vec) & X86_EXC_HAVE_EC)
        /* Push dummy error code (if needed) to align stack. */
        push  $0
        .endif

        /* Store entry vector in the top 32 bits of error_code. */
        movl  $\vec, 4(%rsp)

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

        mov %rsp, %rdi          /* struct cpu_regs * */
        call do_exception

        RESTORE_ALL
        add $8, %rsp            /* Pop error_code/entry_vector. */

        env_IRETQ
ENDFUNC(handle_exception)


ENTRY(entry_ret_to_kernel)      /* int $X86_VEC_RET2KERN */
        env_ADJUST_FRAME

        mov %rbp, %rsp          /* Restore %rsp to exec_user_param()'s context. */
        ret
ENDFUNC(entry_ret_to_kernel)

ENTRY(exec_user_param)          /* ulong (*fn)(ulong), ulong p1 */
        push %rbp

        /* Prepare to "call" exec_user_stub(). */
        push $1f                /* Fake return addr as if we'd called exec_user_stub(). */
        mov %rsp, %rbp          /* Stash %rsp for entry_ret_to_kernel(). */

        /* Prepare an IRET frame. */
        push exec_user_ss(%rip) /* SS */
                                /* RSP */
        push $user_stack + PAGE_SIZE
        pushf                   /* RFLAGS */

        /* Apply and/or masks to eflags. */
        mov exec_user_efl_and_mask(%rip), %rdx
        and %rdx, (%rsp)
        mov exec_user_efl_or_mask(%rip), %rdx
        or %rdx, (%rsp)

        push exec_user_cs(%rip) /* CS */
        push $exec_user_stub    /* RIP */

        env_IRETQ               /* Drop to user privilege. */

1:      /* entry_ret_to_kernel() returns here with a sensible stack. */
        pop %rbp
        ret

ENDFUNC(exec_user_param)

.pushsection .text.user, "ax", @progbits
ENTRY(exec_user_stub)
        xchg %rdi, %rsi         /* Swap p1 to be first parameter to fn(). */
        call *%rsi              /* fn(p1) */

        int $X86_VEC_RET2KERN   /* Return to kernel privilege. */
ENDFUNC(exec_user_stub)
.popsection

ENTRY(entry_EVTCHN)
        env_ADJUST_FRAME

        push $0
        movl $0x200, 4(%rsp)

        SAVE_ALL

        mov %rsp, %rdi          /* struct cpu_regs * */
        call do_evtchn

        RESTORE_ALL
        add $8, %rsp            /* Pop error_code/entry_vector. */

        env_IRETQ
ENDFUNC(entry_EVTCHN)

#if defined(CONFIG_PV)
ENTRY(entry_SYSCALL)
        env_ADJUST_FRAME

        push $0
        movl $0x100, 4(%rsp)

        SAVE_ALL

        mov %rsp, %rdi          /* struct cpu_regs * */
        call do_syscall

        RESTORE_ALL

        movq $VGCF_in_syscall, (%rsp) /* Clobber error_code/entry_vector */
        jmp HYPERCALL_iret

ENDFUNC(entry_SYSCALL)

ENTRY(entry_SYSENTER)
        env_ADJUST_FRAME

        push $0
        movl $0x200, 4(%rsp)

        SAVE_ALL

        mov %rsp, %rdi          /* struct cpu_regs * */
        call do_sysenter

        RESTORE_ALL

        movq $0, (%rsp)         /* Clobber error_code/entry_vector */
        jmp HYPERCALL_iret

ENDFUNC(entry_SYSENTER)
#endif /* CONFIG_PV */

/*
 * Local variables:
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 */
