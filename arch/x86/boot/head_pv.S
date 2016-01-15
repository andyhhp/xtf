#include <xtf/asm_macros.h>

#include <arch/x86/page.h>

#include <xen/elfnote.h>

/* Guest name and version */
ELFNOTE(Xen, XEN_ELFNOTE_GUEST_OS, .asciz "XTF")
ELFNOTE(Xen, XEN_ELFNOTE_GUEST_VERSION, .asciz "0")

/* PV loader */
ELFNOTE(Xen, XEN_ELFNOTE_LOADER, .asciz "generic")
ELFNOTE(Xen, XEN_ELFNOTE_HYPERCALL_PAGE, _WORD hypercall_page)

/* Xen ABI information */
ELFNOTE(Xen, XEN_ELFNOTE_XEN_VERSION, .asciz "xen-3.0")
ELFNOTE(Xen, XEN_ELFNOTE_FEATURES, .asciz "!writable_page_tables|pae_pgdir_above_4gb")
ELFNOTE(Xen, XEN_ELFNOTE_PAE_MODE, .asciz "yes")

        .text
#ifdef __x86_64__
        .code64
#else
        .code32
#endif

 /* PV entry point. */
GLOBAL(_start)

        /* Stash start_info_t pointer from domain build. */
#ifdef __x86_64__
        mov %rsi, start_info
#else
        mov %esi, start_info
#endif

        /* Move onto own stack. */
        mov $boot_stack + PAGE_SIZE, %esp

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
