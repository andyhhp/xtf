#ifndef XTF_LIB_H
#define XTF_LIB_H

#include <xtf/compiler.h>
#include <xtf/console.h>
#include <xtf/types.h>

#if defined(__i386__)
# define BYTES_PER_LONG 4
#elif defined(__x86_64__)
# define BYTES_PER_LONG 8
#else
# errror Bad width
#endif

#define BITS_PER_LONG (BYTES_PER_LONG * 8)


#define ARRAY_SIZE(a)    (sizeof(a) / sizeof(*a))

#define ACCESS_ONCE(x)   (*(volatile typeof(x) *)&(x))

void __noreturn panic(const char *fmt, ...) __printf(1, 2);

#define ASSERT(cond)                                    \
    do { if ( !(cond) )                                 \
            panic("ASSERT(%s) failed at %s:%u\n",       \
                  #cond, __FILE__, __LINE__);           \
    } while ( 0 )

#define BUILD_BUG_ON(cond)                              \
    _Static_assert(!cond, "!(" #cond ")")

#define min(a, b)                                       \
    ({                                                  \
        const typeof(a) _a = (a);                       \
        const typeof(b) _b = (b);                       \
        (void)(&_a == &_b);                             \
        _a < _b ? _a : _b;                              \
    })

#define max(a, b)                                       \
    ({                                                  \
        const typeof(a) _a = (a);                       \
        const typeof(b) _b = (b);                       \
        (void)(&_a == &_b);                             \
        _a > _b ? _a : _b;                              \
    })

#define ROUNDUP(x, a) (((x) + (a) - 1) & ~((a) - 1))

void heapsort(void *base, size_t nmemb, size_t size,
              int (*compar)(const void *, const void *),
              void (*swap)(void *, void *));

/*
 * Execute fn() at user privilege on the current stack, passing its return
 * value back.
 */
unsigned long exec_user(unsigned long (*fn)(void));

/*
 * Wrapper around exec_user() which calls a void function.
 */
static inline void exec_user_void(void (*fn)(void))
{
    exec_user((void *)fn);
}

/**
 * Probe for the SYSCTL_INTERFACE_VERSION in use by the hypervisor
 * @returns version, or -1 on failure.
 */
int xtf_probe_sysctl_interface_version(void);

#endif /* XTF_LIB_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
