/**
 * @file include/arch/x86/symbolic-const.h
 *
 * Macros for creating constants using mnemonics.
 */
#ifndef XTF_X86_SYMBOLIC_CONST_H
#define XTF_X86_SYMBOLIC_CONST_H

#include <xtf/macro_magic.h>

#include <arch/x86/desc.h>

/**
 * Tokenise and OR together.
 *
 * For each varadic, tokenise with 't' and OR together.
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
#define TOK_OR0(t)         (0)
#define TOK_OR1(t, x)      (t ## x)
#define TOK_OR2(t, x, ...) (t ## x | TOK_OR1(t, ##__VA_ARGS__))
#define TOK_OR3(t, x, ...) (t ## x | TOK_OR2(t, ##__VA_ARGS__))
#define TOK_OR4(t, x, ...) (t ## x | TOK_OR3(t, ##__VA_ARGS__))
#define TOK_OR5(t, x, ...) (t ## x | TOK_OR4(t, ##__VA_ARGS__))
#define TOK_OR6(t, x, ...) (t ## x | TOK_OR5(t, ##__VA_ARGS__))
#define TOK_OR7(t, x, ...) (t ## x | TOK_OR6(t, ##__VA_ARGS__))
#define TOK_OR8(t, x, ...) (t ## x | TOK_OR7(t, ##__VA_ARGS__))
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
