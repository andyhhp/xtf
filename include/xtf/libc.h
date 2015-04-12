#ifndef XTF_LIBC_H
#define XTF_LIBC_H

#include <xtf/types.h>

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

size_t strlen(const char *str);
size_t strnlen(const char *str, size_t max);

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
