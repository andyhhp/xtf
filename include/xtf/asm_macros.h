/**
 * @file include/xtf/asm_macros.h
 *
 * Macros for use in assembly files.
 */
#ifndef XTF_ASM_MACROS_H
#define XTF_ASM_MACROS_H

#include <xtf/numbers.h>

#include <arch/asm_macros.h>

#ifdef __ASSEMBLY__

/**
 * Declare a global symbol.
 * @param name Symbol name.
 */
#define GLOBAL(name)                            \
    .globl name;                                \
name:

/**
 * Declare a function entry, aligned on a cacheline boundary.
 * @param name Function name.
 */
#define ENTRY(name)                             \
    ALIGN;                                      \
    GLOBAL(name)

/**
 * Set the size of a named symbol.
 * @param name Symbol name.
 */
#define SIZE(name)                              \
    .size name, . - name;

/**
 * Set the type of @p name to function, and set its size.
 * @param name Symbol name.
 */
#define ENDFUNC(name)                           \
    .type name, STT_FUNC;                       \
    SIZE(name)

/**
 * Declare a string with label @p name and value @p val.  It is placed in the
 * mergable string section, is declared as data, and has its size set.
 * @param name String name.
 * @param val String content.
 */
#define DECLSTR(name, val)                          \
    .pushsection .rodata.str1, "aMS", @progbits, 1; \
    name: .asciz val;                               \
    .type name, STT_OBJECT;                         \
    SIZE(name)                                      \
    .popsection

/**
 * Create an ELF note entry.
 *
 * 'desc' may be an arbitrary asm construct.
 */
#define ELFNOTE(name, type, desc)                   \
    .pushsection .note.name, "a", @note           ; \
    .align 4                                      ; \
    .long 2f - 1f         /* namesz */            ; \
    .long 4f - 3f         /* descsz */            ; \
    .long type            /* type   */            ; \
1:.asciz #name            /* name   */            ; \
2:.align 4                                        ; \
3:desc                    /* desc   */            ; \
4:.align 4                                        ; \
    .popsection

#else

#define ELFNOTE(name, type, desc)                \
    asm (".pushsection .note, \"a\", @note;"     \
    ".align 4;"                                  \
    ".long 2f - 1f;"       /* namesz */          \
    ".long 4f - 3f;"       /* descsz */          \
    ".long " STR(type) ";" /* type   */          \
    "1: .asciz \"" #name "\";" /* name   */      \
    "2:.align 4;"                                \
    "3: " desc ";"         /* desc   */          \
    "4:.align 4;"                                \
    ".popsection;")

#endif /* __ASSEMBLY__ */

#endif /* XTF_ASM_MACROS_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
