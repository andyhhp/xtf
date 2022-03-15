#include <arch/page.h>

#if defined(__arm__)
OUTPUT_ARCH(arm)
OUTPUT_FORMAT("elf32-littlearm")
#elif defined(__aarch64__)
OUTPUT_ARCH(aarch64)
OUTPUT_FORMAT("elf64-littleaarch64")
#endif

ENTRY(_start)

SECTIONS
{
    . = XTF_VIRT_START;
    _text = .;

    .text : {
        *(.text.head)
        *(.text)
    }

    . = ALIGN(PAGE_SIZE);

    .data : {
        *(.data)

    . = ALIGN(PAGE_SIZE);
        *(.data.page_aligned)
    }

    . = ALIGN(PAGE_SIZE);

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }

    . = ALIGN(PAGE_SIZE);

    .bss : {
        __start_bss = .;
        *(.bss)

    . = ALIGN(PAGE_SIZE);
        *(.bss.page_aligned)

        __end_bss = .;
    }

    _end = .;

    /*
     * It is possible for a GNU linker to add a .note.gnu.build-id section
     * before .text which causes zimage header to be shifted resulting in
     * a bad magic. Discard this section to prevent errors.
     */
    /DISCARD/ : { *(.note.gnu.build-id) }
}
