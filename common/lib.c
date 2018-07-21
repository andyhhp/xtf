#include <xtf/lib.h>
#include <xtf/traps.h>
#include <xtf/hypercall.h>
#include <xtf/xenstore.h>

#ifndef isdigit
/* Avoid pulling in all of ctypes just for this. */
static int isdigit(int c)
{
    return c >= '0' && c <= '9';
}
#endif

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

int xtf_probe_domctl_interface_version(void)
{
    int i;
    struct xen_domctl op = { .cmd = 0 };

    for ( i = 0; i < 128; i++ )
    {
        op.interface_version = i;
        if ( hypercall_domctl(&op) != -EACCES )
            return i;
    }

    return -1;
}

int xtf_get_domid(void)
{
    int rc = xenstore_init();

    if ( rc )
        return -1;

    const char *str = xenstore_read("domid");
    unsigned int domid = 0;

    if ( !str || !isdigit(*str) )
        return -1;

    while ( isdigit(*str) )
    {
        domid = domid * 10 + (*str - '0');
        str++;
    }

    if ( domid >= DOMID_FIRST_RESERVED )
        return -1;

    return domid;
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
