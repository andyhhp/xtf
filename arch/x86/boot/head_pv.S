#include <xtf/asm_macros.h>

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
        call xtf_main
        ud2
SIZE(_start)
