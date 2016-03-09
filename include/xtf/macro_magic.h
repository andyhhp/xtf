/**
 * @file include/xtf/macro_magic.h
 *
 * Varadic macro helpers - Here be many dragons.
 */
#ifndef XTF_MACRO_MAGIC_H
#define XTF_MACRO_MAGIC_H

/**
 * Count the number of varadic arguments provided.
 *
 * <pre>
 *   VA_NARGS()     => 0
 *   VA_NARGS(x)    => 1
 *   VA_NARGS(x, y) => 2
 * </pre>
 *
 * Currently functions for 0 to 11 arguments.
 */
/** @cond */
#define VA_NARGS_(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, N, ...) N
/** @endcond */
#define VA_NARGS(...) \
    VA_NARGS_(X,##__VA_ARGS__, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

/**
 * Call a macro variation, based on the number of varadic arguments.
 *
 * @param macro Partial token to call a variation of.
 * @param c1    Constant parameter to pass through.
 * @param ...   Varadic arguments to pass through.
 *
 * Tokenises 'macro' with the count of varadic arguments, passing 'c1' and the
 * varadic arguments.
 *
 * <pre>
 *   VAR_MACRO_C1(m, c)          => m0(c)
 *   VAR_MACRO_C1(m, c, x)       => m1(c, x)
 *   VAR_MACRO_C1(m, c, x, y)    => m2(c, x, y)
 *   VAR_MACRO_C1(m, c, x, y, z) => m3(c, x, y, z)
 * </pre>
 */
/** @cond */
#define VAR_MACRO_C1__(macro, c1, count, ...) macro##count(c1, ##__VA_ARGS__)
#define VAR_MACRO_C1_(macro, c1, count, ...)        \
    VAR_MACRO_C1__(macro, c1, count, ##__VA_ARGS__)
/** @endcond */
#define VAR_MACRO_C1(macro, c1, ...)                                \
    VAR_MACRO_C1_(macro, c1, VA_NARGS(__VA_ARGS__), ##__VA_ARGS__)

/**
 * Evalute whether the CONFIG_ token @p x is defined
 *
 * Suitable for use in an `if ()` condition, even if @p x is not defined.
 * Will strictly evaluate to 0 or 1.
 */
#define IS_DEFINED(x) _IS_DEFINED(x)
/** @cond */
#define _IS_DEFINED_PARTIAL_1 0,
#define _IS_DEFINED(x) __IS_DEFINED(_IS_DEFINED_PARTIAL_ ## x)
#define __IS_DEFINED(y) ___IS_DEFINED(y 1, 0)
#define ___IS_DEFINED(maybe, val, ...) val
/** @endcond */

#endif /* XTF_MACRO_MAGIC_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
