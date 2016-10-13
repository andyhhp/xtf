/**
 * @file include/arch/x86/symbolic-const.h
 *
 * Macros for creating constants using mnemonics.
 */
#ifndef XTF_X86_SYMBOLIC_CONST_H
#define XTF_X86_SYMBOLIC_CONST_H

#include <xtf/macro_magic.h>

#include <arch/x86/desc.h>
#include <arch/x86/processor.h>

/**
 * Tokenise and OR together.
 *
 * For each varadic parameter, tokenise with 't' and OR together.
 *
 * @param t   Common stem partial token.
 * @param ... Partial tokens.
 *
 * Example:
 * <pre>
 *   TOK_OR(t, x, y)    => (t ## x | t ## y)
 *   TOK_OR(t, x, y, z) => (t ## x | t ## y | t ## z)
 * </pre>
 */
/** @cond */
#define TOK_OR0(t)          (0)
#define TOK_OR1(t, x)       (t ## x)
#define TOK_OR2(t, x, ...)  (t ## x | TOK_OR1(t, ##__VA_ARGS__))
#define TOK_OR3(t, x, ...)  (t ## x | TOK_OR2(t, ##__VA_ARGS__))
#define TOK_OR4(t, x, ...)  (t ## x | TOK_OR3(t, ##__VA_ARGS__))
#define TOK_OR5(t, x, ...)  (t ## x | TOK_OR4(t, ##__VA_ARGS__))
#define TOK_OR6(t, x, ...)  (t ## x | TOK_OR5(t, ##__VA_ARGS__))
#define TOK_OR7(t, x, ...)  (t ## x | TOK_OR6(t, ##__VA_ARGS__))
#define TOK_OR8(t, x, ...)  (t ## x | TOK_OR7(t, ##__VA_ARGS__))
#define TOK_OR9(t, x, ...)  (t ## x | TOK_OR8(t, ##__VA_ARGS__))
#define TOK_OR10(t, x, ...) (t ## x | TOK_OR9(t, ##__VA_ARGS__))
#define TOK_OR11(t, x, ...) (t ## x | TOK_OR10(t, ##__VA_ARGS__))
/** @endcond */
#define TOK_OR(t, ...)     VAR_MACRO_C1(TOK_OR, t, ##__VA_ARGS__)

/**
 * Initialise an LDT/GDT entry using SEG_ATTR_ mnemonics.
 *
 * @param base  Segment base.
 * @param limit Segment limit.
 * @param ...   Partial SEG_ATTR_ tokens for attributes.
 *
 * Example usage:
 * - INIT_GDTE_SYM(0, 0xfffff, P)
 *   - uses @ref SEG_ATTR_P
 *
 * - INIT_GDTE_SYM(0, 0xfffff, CODE, L)
 *   - uses @ref SEG_ATTR_CODE and @ref SEG_ATTR_L
 */
#define INIT_GDTE_SYM(base, limit, ...) \
    INIT_GDTE(base, limit, TOK_OR(SEG_ATTR_, ##__VA_ARGS__))

/**
 * Create a selector based error code using X86_EC_ mnemonics.
 *
 * @param sel Selector value.
 * @param ... Partial X86_EC_ tokens.
 *
 * Example usage:
 * - EXC_EC_SYM(0, GDT)
 *   - Uses @ref X86_EC_GDT.
 *
 * - EXC_EC_SYM(0, IDT, EXT)
 *   - Uses @ref X86_EC_IDT and @ref X86_EC_EXT.
 */
#define SEL_EC_SYM(sel, ...) (sel | TOK_OR(X86_EC_, ##__VA_ARGS__))

/**
 * Create an exception selector based error code using mnemonics, with
 * implicit @ref X86_EC_IDT.
 *
 * @param exc Partial X86_EXC_ token for selector.
 * @param ... Partial X86_EC_ tokens.
 *
 * Example usage:
 * - EXC_EC_SYM(DE)
 *   - Uses @ref X86_EXC_DE and @ref X86_EC_IDT.
 *
 * - EXC_EC_SYM(DB, EXT)
 *   - Uses @ref X86_EXC_DB, @ref X86_EC_IDT and @ref X86_EC_EXT.
 */
#define EXC_EC_SYM(exc, ...) \
    SEL_EC_SYM(((X86_EXC_ ## exc) << 3), IDT, ##__VA_ARGS__)

/**
 * Create pagetable entry flags based on mnemonics.
 *
 * @param ... Partial _PAGE_ tokens.
 *
 * Example usage:
 * - PF_SYM(AD, U, RW, P)
 *   - Accessed, Dirty, User, Writeable, Present.
 */
#define PF_SYM(...) TOK_OR(_PAGE_, ##__VA_ARGS__)

#endif /* XTF_X86_SYMBOLIC_CONST_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
