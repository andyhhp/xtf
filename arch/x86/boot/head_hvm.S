#include <xtf/asm_macros.h>

#include <arch/x86/page.h>
#include <arch/x86/processor.h>
#include <arch/x86/msr-index.h>
#include <arch/x86/segment.h>

        .code32 /* Always starts in 32bit flat mode. */


/* HVM common setup for pae paging. */
GLOBAL(_start)

        /* CR4.PAE = 1 */
        mov $X86_CR4_PAE, %eax
        mov %eax, %cr4

        /* CR3 = l?_???map */
#ifdef __x86_64__
        mov $l4_identmap, %eax
#else
        mov $l3_paemap, %eax
#endif
        mov %eax, %cr3

#ifdef __x86_64__
        /* EFER.LME = 1 */
        mov $MSR_EFER, %ecx
        rdmsr
        bts $_EFER_LME, %eax
        wrmsr
#endif

        /* CR0.{PG,PE} = 1 */
        mov %cr0, %eax
        or $(X86_CR0_PG | X86_CR0_PE), %eax
        mov %eax, %cr0

        lgdt gdt_ptr

        /* Load code segment. */
        ljmp $__KERN_CS, $1f
#ifdef __x86_64__
        .code64
#endif

        /* Load data segments. */
1:      mov $__USER_DS, %eax
        mov %eax, %ds
        mov %eax, %es
        mov %eax, %fs
        mov %eax, %gs
        mov $__KERN_DS, %eax
        mov %eax, %ss

        /* Move onto the boot stack. */
        mov $boot_stack + PAGE_SIZE, %esp

        /* Reset flags. */
        push $X86_EFLAGS_MBS
        popf

        call xtf_main

        /* panic() if xtf_main manages to return. */
#ifdef __x86_64__
        lea main_err_msg(%rip), %rdi
#else
        push $main_err_msg
#endif
        call panic
SIZE(_start)

.section .rodata.str1, "aMS", @progbits, 1

main_err_msg: .asciz "xtf_main() returned\n"
SIZE(main_err_msg)
