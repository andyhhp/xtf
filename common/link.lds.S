/*
 * Common linker file for all architectures/environments
 */

#include <xtf/numbers.h>
#include <arch/page.h>

/*
 * Architecture must provide:
 *   - OUTPUT_FORMAT()
 *   - OUTPUT_ARCH()
 *   - ENTRY()
 */
#define LINKER_HEADER
#include <arch/link.lds.h>
#undef LINKER_HEADER

PHDRS
{
        text PT_LOAD FLAGS(7); /* RWE */
        note PT_NOTE FLAGS(4); /* R   */
}

SECTIONS
{
        . = XTF_VIRT_START;

        _start = .;

        .text : {
                *(.text.head)
                *(.text)

        . = ALIGN(PAGE_SIZE);
        __start_user_text = .;
                *(.text.user)
        . = ALIGN(PAGE_SIZE);
        __end_user_text = .;

        } :text = 0

        .data : {
                *(.data)
        . = ALIGN(PAGE_SIZE);
                *(.data.page_aligned)
        . = ALIGN(PAGE_SIZE);

        __start_user_data = .;
                *(.data.user)
        . = ALIGN(PAGE_SIZE);
        __end_user_data = .;

        }

        .note : {
                *(.note)
                *(.note.*)
        } :note :text

        .rodata : {
                *(.rodata)
                *(.rodata.*)

        . = ALIGN(8);
        __start_ex_table = .;
                *(.ex_table)
        __stop_ex_table = .;
        } :text

        .bss : {
                *(.bss)
        . = ALIGN(PAGE_SIZE);
                *(.bss.page_aligned)
        . = ALIGN(PAGE_SIZE);

        __start_user_bss = .;
                *(.bss.user.page_aligned)
        . = ALIGN(PAGE_SIZE);
        __end_user_bss = .;
        }

        _end = .;
}

/*
 * Linker games to use virtual addresses at 0 without triggering NULL pointer
 * "logic" and associated compiler optimisations.
 */
zeroptr = 0;

ASSERT(IS_ALIGNED(__start_user_text, PAGE_SIZE), "__start_user_text misaligned");
ASSERT(IS_ALIGNED(__end_user_text,   PAGE_SIZE), "__end_user_text misaligned");
ASSERT(IS_ALIGNED(__start_user_data, PAGE_SIZE), "__start_user_data misaligned");
ASSERT(IS_ALIGNED(__end_user_data,   PAGE_SIZE), "__end_user_data misaligned");
ASSERT(IS_ALIGNED(__start_user_bss,  PAGE_SIZE), "__start_user_data misaligned");
ASSERT(IS_ALIGNED(__end_user_bss,    PAGE_SIZE), "__end_user_data misaligned");

/* Architecture may provide any extra asserts. */
#define LINKER_FOOTER
#include <arch/link.lds.h>
#undef LINKER_FOOTER

/*
 * Local variables:
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 */
