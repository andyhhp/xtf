/**
 * @file include/inttypes.h
 *
 * Local subset of C's inttypes.h
 */
#ifndef INTTYPES_H
#define INTTYPES_H

#if __SIZEOF_LONG__ == 8
# define __PRI64  "l"
# define __PRIPTR "l"
#else
# define __PRI64  "ll"
# define __PRIPTR
#endif

#define PRId32              "d"
#define PRIx32              "x"
#define PRIo32              "o"
#define PRIu32              "u"

#define PRId64     __PRI64  "d"
#define PRIx64     __PRI64  "x"
#define PRIo64     __PRI64  "o"
#define PRIu64     __PRI64  "u"

#define PRIdPTR    __PRIPTR "d"
#define PRIoPTR    __PRIPTR "o"
#define PRIuPTR    __PRIPTR "u"
#define PRIxPTR    __PRIPTR "x"

#endif /* INTTYPES_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
