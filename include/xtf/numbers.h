/**
 * @file include/xtf/numbers.h
 *
 * Primatives for number manipulation.
 */
#ifndef XTF_NUMBERS_H
#define XTF_NUMBERS_H

/**
 * Create an integer usable in both C and Assembly, even when @p suf is
 * needed.
 */
#ifdef __ASSEMBLY__
# define _AC(num, suf) num
#else
# define _AC(num, suf) __tok(num, suf)
/** @cond */
# define __tok(a, b)   (a ## b)
/** @endcond */
#endif

/** Express @p num in Kilobytes. */
#define KB(num) (_AC(num, ULL) << 10)

/** Express @p num in Megabytes. */
#define MB(num) (_AC(num, ULL) << 20)

/** Express @p num in Gigabytes. */
#define GB(num) (_AC(num, ULL) << 30)

/** Express @p num in Terabytes. */
#define TB(num) (_AC(num, ULL) << 40)

/**
 * Return a boolean indicating whether @p val is aligned on @p align.  @p
 * align is required to be exact power of two.
 */
#define IS_ALIGNED(val, align) (((val) & ((align) - 1)) == 0)

#define MASK_EXTR(v, m) (((v) & (m)) / ((m) & -(m)))
#define MASK_INSR(v, m) (((v) * ((m) & -(m))) & (m))

#ifndef __ASSEMBLY__

/**
 * Express an abitrary integer @p v as void *.
 */
#define _p(v) ((void*)(unsigned long)(v))

/**
 * Express an arbitrary value @p v as unsigned long.
 */
#define _u(v) ((unsigned long)(v))

/**
 * Round up a number to the next integer
 */
#define roundup(x, y) ((((x) + ((y) - 1)) / (y)) * (y))

#endif /* !__ASSEMBLY__ */
#endif /* XTF_NUMBERS_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
