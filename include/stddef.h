/**
 * @file include/stddef.h
 *
 * Local subset of C's stddef.h
 */
#ifndef STDDEF_H
#define STDDEF_H

typedef __SIZE_TYPE__       size_t;
typedef __PTRDIFF_TYPE__    ptrdiff_t;

#define NULL ((void *)0)

#define offsetof(t, m) __builtin_offsetof(t, m)

#endif /* STDDEF_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
