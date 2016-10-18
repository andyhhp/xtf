/*
 * Common linker file for all x86 environments
 */

#include <xtf/numbers.h>
#include <arch/page.h>

/* Don't clobber the ld directive */
#undef i386

#if defined(__x86_64__)

OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)

#elif defined(__i386__)

OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)

#else
# error Bad architecture to link with
#endif

ENTRY(_start)

PHDRS
{
        /* Single loadable section, RWE. */
        load PT_LOAD FLAGS(7);
}

SECTIONS
{
        . = SEGMENT_START("text-segment", MB(1));

        .text : {
                *(.text)

	. = ALIGN(PAGE_SIZE);
        __start_user_text = .;
                *(.text.user)
	. = ALIGN(PAGE_SIZE);
        __end_user_text = .;

        } :load = 0

        .data : {
                *(.data)
        . = ALIGN(PAGE_SIZE);
                *(.data.page_aligned)
        . = ALIGN(PAGE_SIZE);
        }

        .rodata : {
                *(.rodata)
                *(.rodata.*)

        . = ALIGN(8);
        __start_ex_table = .;
                *(.ex_table)
        __stop_ex_table = .;
        }

        .note : {
                *(.note)
                *(.note.*)
        }

        .bss : {
                *(.bss)
        . = ALIGN(PAGE_SIZE);
                *(.bss.page_aligned)
        . = ALIGN(PAGE_SIZE);
        }

        _end = .;
}

/*
 * Linker games to use virtual addresses at 0 without triggering NULL pointer
 * "logic" and associated compiler optimisations.
 */
zeroptr = 0;

ASSERT(IS_ALIGNED(hypercall_page, PAGE_SIZE), "hypercall_page misaligned");
ASSERT(IS_ALIGNED(boot_stack, PAGE_SIZE), "boot_stack misaligned");
ASSERT(IS_ALIGNED(user_stack, PAGE_SIZE), "user_stack misaligned");

ASSERT(IS_ALIGNED(__start_user_text, PAGE_SIZE), "__start_user_text misaligned");
ASSERT(IS_ALIGNED(__end_user_text,   PAGE_SIZE), "__end_user_text misaligned");

#ifdef CONFIG_HVM

ASSERT(IS_ALIGNED(pae_l1_identmap, PAGE_SIZE), "pae_l1_identmap misaligned");
ASSERT(IS_ALIGNED(pae_l2_identmap, PAGE_SIZE), "pae_l2_identmap misaligned");
ASSERT(IS_ALIGNED(pae_l3_identmap, PAGE_SIZE), "pae_l3_identmap misaligned");
ASSERT(IS_ALIGNED(pae_l4_identmap, PAGE_SIZE), "pae_l4_identmap misaligned");

ASSERT(IS_ALIGNED(pae32_l3_identmap, 32), "pae32_l3_ident misaligned");

ASSERT(IS_ALIGNED(pse_l1_identmap, PAGE_SIZE), "pse_l1_identmap misaligned");
ASSERT(IS_ALIGNED(pse_l2_identmap, PAGE_SIZE), "pse_l2_identmap misaligned");

#endif
/*
 * Local variables:
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 */
