/*
 * Common linker file for all x86 environments
 */

#include <xtf/numbers.h>

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
        } :load = 0x9090

        .rodata : {
                *(.rodata)
                *(.rodata.*)
        }

        .note : {
                *(.note)
                *(.note.*)
        }
}
