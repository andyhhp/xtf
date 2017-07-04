/**
 * @file include/xtf/types.h
 *
 * Common declarations for all tests.
 */
#ifndef XTF_TYPES_H
#define XTF_TYPES_H

#ifndef __ASSEMBLY__

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <inttypes.h>
#include <limits.h>

/**
 * An array which the linker resolves to 0.
 *
 * For use instead of NULL when access to 0 is really needed, without
 * triggering NULL pointer logic in the compiler.
 */
extern char zeroptr[];

#endif /* !__ASSEMBLY__ */

#endif /* XTF_TYPES_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
