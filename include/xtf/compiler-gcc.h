#ifndef XTF_COMPILER_GCC_H
#define XTF_COMPILER_GCC_H

#define GCC_VER (__GNUC__ * 10000 +             \
                 __GNUC_MINOR__ * 100 +         \
                 __GNUC_PATCHLEVEL__)

/*
 * The Clang __has_*() infrastructure is a very clean way to identify
 * compiler support, without resorting to version checks.  Fake up
 * enough support for XTF code to use, even on non-clang compilers.
 */

#ifndef __has_extension

#define GCC_HAS_c_static_assert (GCC_VER >= 40600) /* _Static_assert() */

#define __has_extension(x) GCC_HAS_ ## x
#endif /* __has_extension */

#endif /* XTF_COMPILER_GCC_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
