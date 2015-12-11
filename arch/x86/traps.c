#include <xtf/lib.h>
#include <xtf/traps.h>

/*
 * C entry-point for exceptions, after the per-environment stubs have suitably
 * adjusted the stack.
 */
void do_exception(void)
{
    panic("Unhandled exception\n");
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
