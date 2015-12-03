/*
 * Common linker file for all x86 environments
 */

#include <xtf/numbers.h>

/* Don't clobber the ld directive */
#undef i386

#if defined(__x86_64__)

#if defined(CONFIG_ENV_hvm64)

/*
 * All HVM guests start in 32bit mode in a stub
 * which switches to the desired running mode.
 */
OUTPUT_FORMAT("elf32-x86-64")

#else

OUTPUT_FORMAT("elf64-x86-64")

#endif

OUTPUT_ARCH(i386:x86-64)

#elif defined(__i386__)

OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)

#else
# error Bad architecture to link with
#endif

ENTRY(_start)

SECTIONS
{
        . = 0x10000;

        .text : {
                *(.text)
        } = 0x9090

        .rodata : {
                *(.rodata)
                *(.rodata.*)
        }

        .note : {
                *(.note)
                *(.note.*)
        }
}
