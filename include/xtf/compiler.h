#ifndef XTF_COMPILER_H
#define XTF_COMPILER_H

#ifdef __GNUC__
#include <xtf/compiler-gcc.h>
#endif

#define __alias(x)            __attribute__((__alias__(x)))
#define __aligned(x)          __attribute__((__aligned__(x)))
#define __noreturn            __attribute__((__noreturn__))
#define __packed              __attribute__((__packed__))
#define __printf(f, v)        __attribute__((__format__(__printf__, f, v)))
#define __maybe_unused        __attribute__((__unused__))
#define __transparent         __attribute__((__transparent_union__))
#define __used                __attribute__((__used__))
#define __weak                __attribute__((__weak__))

#ifndef __noinline /* Avoid conflicting with cdefs.h */
#define __noinline            __attribute__((__noinline__))
#endif

#ifndef __always_inline /* Avoid conflicting with cdefs.h */
#define __always_inline       __attribute__((__always_inline__))
#endif

#ifndef __section /* Avoid conflicting with cdefs.h */
#define __section(s)          __attribute__((__section__(s)))
#endif

#define unreachable()         __builtin_unreachable()
#define barrier()             __asm__ __volatile__ ("" ::: "memory")

/* Convenience wrappers. */
#define __user_text           __section(".text.user")
#define __user_data           __section(".data.user")

#define __page_aligned_data   __section(".data.page_aligned") __aligned(4096)
#define __page_aligned_bss    __section(".bss.page_aligned")  __aligned(4096)

#define __user_page_aligned_bss \
    __section(".bss.user.page_aligned")  __aligned(4096)

#endif /* XTF_COMPILER_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
