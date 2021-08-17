/*
 * @file common/weak-defaults.c
 *
 * Default implementations of functions called by common code, which may need
 * arch or environment specific implementations.
 */
#include <xtf/framework.h>
#include <xtf/types.h>

const char __weak environment_description[] = "Unknown";

void __weak arch_setup(void)
{
}

void __weak test_setup(void)
{
}

bool __weak arch_fmt_pointer(
    char **str, char *end, const char **fmt_ptr, const void *arg,
    int width, int precision, unsigned int flags)
{
    return false;
}

void __weak __noreturn arch_crash_hard(void)
{
    /* panic() has failed.  Sit in a tight loop. */
    for ( ;; )
        ;
    unreachable();
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
