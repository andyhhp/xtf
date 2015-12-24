#include <arch/x86/processor.h>
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
#if defined(CONFIG_ENV_pv64)
                                /* Restore results of Xen SYSRET'ing to this point. */
        pop   %rcx
        pop   %r11
#endif
.endm

.macro env_IRETQ                /* Environment specific version of `iretq`. */
#if defined(CONFIG_ENV_pv64)

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
