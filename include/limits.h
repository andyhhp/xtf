/**
 * @file include/limits.h
 *
 * Local subset of C's limits.h
 */
#ifndef LIMITS_H
#define LIMITS_H

#define CHAR_BIT __CHAR_BIT__

#define BYTES_PER_LONG __SIZEOF_LONG__
#define BITS_PER_LONG (BYTES_PER_LONG * CHAR_BIT)

#endif /* LIMITS_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
