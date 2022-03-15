#ifndef XTF_LIB_H
#define XTF_LIB_H

#include <xtf/compiler.h>
#include <xtf/console.h>
#include <xtf/types.h>

#define ARRAY_SIZE(a)    (sizeof(a) / sizeof(*a))

#define ACCESS_ONCE(x)   (*(volatile typeof(x) *)&(x))

void __noreturn panic(const char *fmt, ...) __printf(1, 2);

#define ASSERT(cond)                                    \
    do { if ( !(cond) )                                 \
            panic("ASSERT(%s) failed at %s:%u\n",       \
                  #cond, __FILE__, __LINE__);           \
    } while ( 0 )

#if __has_extension(c_static_assert)
# define BUILD_BUG_ON(cond)                             \
    _Static_assert(!(cond), "!(" #cond ")")
#else
# define BUILD_BUG_ON(cond)                             \
    ((void)sizeof(struct { char: -!!(cond); }))
#endif

#define UNIMPLEMENTED() do {                            \
    panic("Unimplemented function -> %s:%u\n",          \
          __FILE__, __LINE__);                          \
} while(0)

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

/**
 * Execute fn(p1) at user privilege, passing its return value back.
 *
 * fn() is executed on user_stack[], and is non-reentrant.
 */
unsigned long exec_user_param(unsigned long (*fn)(unsigned long),
                              unsigned long p1);

/*
 * Wrapper around exec_user_param(), calling a function which takes no
 * parameters.  p1 is poisioned to catch misuses.
 */
static inline unsigned long exec_user(unsigned long (*fn)(void))
{
    return exec_user_param((void *)fn, 0xdead0000);
}

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

/**
 * Obtain the current domid.
 * @returns domid, or -1 on failure.
 */
int xtf_get_domid(void);

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
