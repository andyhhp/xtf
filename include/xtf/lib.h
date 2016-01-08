#ifndef XTF_LIB_H
#define XTF_LIB_H

#include <xtf/types.h>
#include <xtf/compiler.h>
#include <xtf/console.h>
#include <xtf/exlog.h>
#include <xtf/libc.h>
#include <xtf/numbers.h>
#include <xtf/report.h>
#include <xtf/test.h>
#include <xtf/traps.h>

#include <arch/x86/lib.h>

#define ARRAY_SIZE(a)    (sizeof(a) / sizeof(*a))

#define ACCESS_ONCE(x)   (*(volatile typeof(x) *)&(x))

void __noreturn panic(const char *fmt, ...) __printf(1, 2);

void heapsort(void *base, size_t nmemb, size_t size,
              int (*compar)(const void *, const void *),
              void (*swap)(void *, void *));

/* Execute fn() at user privilege on the current stack. */
void exec_user(void (*fn)(void));

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
