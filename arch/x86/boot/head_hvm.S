#include <xtf/asm_macros.h>

#include <arch/x86/page.h>
#include <arch/x86/processor.h>
#include <arch/x86/msr-index.h>
#include <arch/x86/segment.h>

        .code32                 /* Always starts in 32bit flat mode. */
GLOBAL(_start)                  /* HVM common setup. */

#if CONFIG_PAGING_LEVELS > 0    /* Paging setup for CR3 and CR4 */

#if CONFIG_PAGING_LEVELS == 4   /* 64bit PAE, 4 levels */

        mov $X86_CR4_PAE, %eax
        mov $l4_identmap, %ebx

#elif CONFIG_PAGING_LEVELS == 3 /* 32bit PAE, 3 levels */

        mov $X86_CR4_PAE, %eax
        mov $l3_paemap, %ebx

#else
# error Bad paging mode
#endif

        mov %eax, %cr4
        mov %ebx, %cr3
#endif /* CONFIG_PAGING_LEVELS > 0 */

#ifdef __x86_64__               /* EFER.LME = 1 */
        mov $MSR_EFER, %ecx
        rdmsr
        bts $_EFER_LME, %eax
        wrmsr
#endif /* __x86_64__ */

#if CONFIG_PAGING_LEVELS > 0    /* CR0.PG = 1 */
        mov %cr0, %eax
        or $X86_CR0_PG, %eax
        mov %eax, %cr0
#endif /* CONFIG_PAGING_LEVELS > 0 */

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

/*
 * Local variables:
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 */
