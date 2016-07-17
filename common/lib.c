#include <xtf/lib.h>
#include <xtf/traps.h>
#include <xtf/hypercall.h>

void __noreturn panic(const char *fmt, ...)
{
    va_list args;

    printk("******************************\n");

    printk("PANIC: ");
    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);

    printk("******************************\n");

    hypercall_shutdown(SHUTDOWN_crash);
    arch_crash_hard();
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
