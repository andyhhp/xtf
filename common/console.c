#include <xtf/types.h>
#include <xtf/console.h>
#include <xtf/lib.h>
#include <xtf/libc.h>

/*
 * Output functions, registered if/when available.
 * Possibilities:
 */
static cons_output_cb output_fns[1];
static unsigned int nr_cons_cb;

void register_console_callback(cons_output_cb fn)
{
    if ( nr_cons_cb < ARRAY_SIZE(output_fns) )
        output_fns[nr_cons_cb++] = fn;
}

void printk(const char *fmt, ...)
{
    unsigned int i;

    for ( i = 0; i < nr_cons_cb; ++i )
        output_fns[i](fmt, strlen(fmt));
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
