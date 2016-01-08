/**
 * @file include/arch/x86/symbolic-const.h
 *
 * Macros for creating constants using mnemonics.
 */
#ifndef XTF_X86_SYMBOLIC_CONST_H
#define XTF_X86_SYMBOLIC_CONST_H

#include <xtf/macro_magic.h>

#include <arch/x86/desc.h>

/* Macro magic to expand symbolic SEG_ATTR names into a constant */
#define _GDTE_ATTR0()       (0)
#define _GDTE_ATTR1(x)      (SEG_ATTR_ ## x)
#define _GDTE_ATTR2(x, ...) (SEG_ATTR_ ## x | _GDTE_ATTR1(__VA_ARGS__))
#define _GDTE_ATTR3(x, ...) (SEG_ATTR_ ## x | _GDTE_ATTR2(__VA_ARGS__))
#define _GDTE_ATTR4(x, ...) (SEG_ATTR_ ## x | _GDTE_ATTR3(__VA_ARGS__))
#define _GDTE_ATTR5(x, ...) (SEG_ATTR_ ## x | _GDTE_ATTR4(__VA_ARGS__))
#define _GDTE_ATTR6(x, ...) (SEG_ATTR_ ## x | _GDTE_ATTR5(__VA_ARGS__))
#define _GDTE_ATTR7(x, ...) (SEG_ATTR_ ## x | _GDTE_ATTR6(__VA_ARGS__))
#define _GDTE_ATTR8(x, ...) (SEG_ATTR_ ## x | _GDTE_ATTR7(__VA_ARGS__))

#define _GDTE_ATTR(...) VAR_MACRO(_GDTE_ATTR, __VA_ARGS__)

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
    INIT_GDTE(base, limit, _GDTE_ATTR(__VA_ARGS__))

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
