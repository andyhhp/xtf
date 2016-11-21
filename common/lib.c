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

int xtf_probe_sysctl_interface_version(void)
{
    int i;
    xen_sysctl_t op = { .cmd = 0 };

    for ( i = 0; i < 128; i++ )
    {
        op.interface_version = i;
        if ( hypercall_sysctl(&op) != -EACCES )
            return i;
    }

    return -1;
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
