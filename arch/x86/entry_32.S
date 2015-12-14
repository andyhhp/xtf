#include <arch/x86/processor.h>
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

.macro exception_entry sym vec

ENTRY(entry_\sym)

        .if !((1 << \vec) & X86_EXC_HAVE_EC)
        /* Push dummy error code (if needed) to align stack. */
        push  $0
        .endif

        /* Push metadata (entry vector). */
        push  $\vec

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

        push %esp               /* struct cpu_regs * */
        call do_exception
        add $4, %esp
        ud2a
