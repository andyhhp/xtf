#include <xtf/asm_macros.h>

#include <arch/page.h>
#include <arch/processor.h>
#include <arch/msr-index.h>
#include <arch/segment.h>

#include <xen/elfnote.h>

        .code32                 /* Always starts in 32bit flat mode. */
	.text

#define MULTIBOOT_HEADER_MAGIC         0x1BADB002
#define MULTIBOOT_HEADER_MODS_ALIGNED  0x00000001
#define MULTIBOOT_HEADER_FLAGS (MULTIBOOT_HEADER_MODS_ALIGNED | 2)
GLOBAL(multiboot1_header)
        /* Magic number indicating a Multiboot header. */
        .long   MULTIBOOT_HEADER_MAGIC
        /* Flags to bootloader (see Multiboot spec). */
        .long   MULTIBOOT_HEADER_FLAGS
        /* Checksum: must be the negated sum of the first two fields. */
        .long   -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
ENDDATA(multiboot1_header)

ENTRY(elf_entry)                /* HVM common setup. */

#if CONFIG_PAGING_LEVELS > 0    /* Paging setup for CR3 and CR4 */

        /* Set _PAGE_USER on leaf mappings if a test wants them. */
        cmpb $0, test_wants_user_mappings
        je .Lskip_user

        mov $l1_identmap, %edx
        mov $l2_identmap_end, %ecx
        sub %edx, %ecx
        shr $PTE_ORDER, %ecx
        add $PTE_SIZE, %edx /* Avoid setting _PAGE_USER in the NULL entry. */
.Lnext_pte:
        orb $_PAGE_USER, (%edx)
        add $PTE_SIZE, %edx
        loop .Lnext_pte
.Lskip_user:

#if CONFIG_PAGING_LEVELS == 2
        mov $X86_CR4_PSE, %eax
#elif CONFIG_PAGING_LEVELS == 3 || CONFIG_PAGING_LEVELS == 4
        mov $X86_CR4_PAE, %eax
#else
# error Bad paging mode
#endif
        mov %eax, %cr4

        mov $cr3_target, %ebx
        mov %ebx, %cr3
#endif /* CONFIG_PAGING_LEVELS > 0 */

#ifdef __x86_64__               /* EFER.LME = 1 */
        mov $MSR_EFER, %ecx
        rdmsr
        or  $EFER_LME, %eax
        wrmsr
#endif /* __x86_64__ */

#if CONFIG_PAGING_LEVELS > 0    /* CR0.PG = 1 */
# define MAYBE_PG X86_CR0_PG
#else
# define MAYBE_PG 0
#endif /* CONFIG_PAGING_LEVELS > 0 */

        mov %cr0, %eax
        or $(X86_CR0_WP | MAYBE_PG), %eax
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
        lea .Lmain_err_msg(%rip), %rdi
#else
        push $.Lmain_err_msg
#endif
        call panic
ENDFUNC(elf_entry)

DECLSTR(.Lmain_err_msg, "xtf_main() returned\n")

/* All HVM XTF guests are compatible with the PVH ABI. */
ENTRY(pvh_entry)
        mov %ebx, pvh_start_info
        jmp elf_entry
ENDFUNC(pvh_entry)
ELFNOTE(Xen, XEN_ELFNOTE_PHYS32_ENTRY, .long pvh_entry)

/*
 * Local variables:
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 */
