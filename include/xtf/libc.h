#ifndef XTF_LIBC_H
#define XTF_LIBC_H

#include <xtf/types.h>
#include <xtf/compiler.h>

/*
 * Local declaration of bits of libc
 *
 * Use __builtin_???() wherever possible, to allow gcc to perform certain
 * optimisations (e.g. constant folding) otherwise prevented by -fno-builtin.
 *
 * Where optimisations are not possible, the __builtin_???() varient will emit
 * a call to ???(), which needs implementing in common/libc/
 */

#define strlen(s)                   __builtin_strlen(s)
#define strcmp(s1, s2)              __builtin_strcmp(s1, s2)
#define memset(d, c, n)             __builtin_memset(d, c, n)
#define memcpy(d, s, n)             __builtin_memcpy(d, s, n)
#define memcmp(s1, s2, n)           __builtin_memcmp(s1, s2, n)

size_t strlen(const char *str);
size_t strnlen(const char *str, size_t max);
int strcmp(const char *s1, const char *s2);
void *memset(void *s, int c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

int __printf(3, 0)
    vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

static inline int __printf(3, 4)
    snprintf(char *buf, size_t size, const char *fmt, ...)
{
    va_list args;
    int rc;

    va_start(args, fmt);
    rc = vsnprintf(buf, size, fmt, args);
    va_end(args);

    return rc;
}

#endif /* XTF_LIBC_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
