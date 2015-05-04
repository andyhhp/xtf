#ifndef XTF_MACRO_MAGIC_H
#define XTF_MACRO_MAGIC_H

/* Here be many dragons */

/*
 * VA_NRARGS(...) will count the number of provided arguments.
 *
 * - VA_NRARGS()  => 0
 * - VA_NRARGS(x) => 1
 *
 * Currently functions for 0 to 11 arguments.
 */
#define VA_NARGS_(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, N, ...) N
#define VA_NARGS(...) \
    VA_NARGS_(X,##__VA_ARGS__, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

/*
 * VAR_MACRO(macro, ...) will construct a macro which counts its arguments and
 * calls a evaluates a varient of its first parameter.
 *
 * - VAR_MACRO(x)       => x0()
 * - VAR_MACRO(x, y)    => x1(y)
 * - VAR_MACRO(x, y, x) => x2(y, z)
 */
#define VAR_MACRO__(macro, count, ...) macro##count(__VA_ARGS__)
#define VAR_MACRO_(macro, count, ...)  VAR_MACRO__(macro, count, __VA_ARGS__)
#define VAR_MACRO(macro, ...) \
    VAR_MACRO_(macro, VA_NARGS(__VA_ARGS__), __VA_ARGS__)

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
