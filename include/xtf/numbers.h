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
# define __tok(a, b)   (a ## b)
#endif

#define KB(num) (_AC(num, ULL) << 10)
#define MB(num) (_AC(num, ULL) << 20)
#define GB(num) (_AC(num, ULL) << 30)
#define TB(num) (_AC(num, ULL) << 40)

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
