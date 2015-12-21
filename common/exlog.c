#include <xtf/lib.h>
#include <xtf/exlog.h>

static bool logging = false;
static unsigned int log_entries;

static exlog_entry_t log[8];

void xtf_exlog_start(void)
{
    logging = true;
    log_entries = 0;
}

void xtf_exlog_reset(void)
{
    log_entries = 0;
}

void xtf_exlog_stop(void)
{
    logging = false;
}


unsigned int xtf_exlog_entries(void)
{
    return log_entries;
}

exlog_entry_t *xtf_exlog_entry(unsigned int idx)
{
    if ( idx < ARRAY_SIZE(log) && idx < log_entries )
        return &log[idx];
    return NULL;
}

void xtf_exlog_log_exception(struct cpu_regs *regs)
{
    if ( !logging )
        return;

    if ( log_entries < ARRAY_SIZE(log) )
    {
        exlog_entry_t *e = &log[log_entries];

        e->ip = regs->ip;
        e->cs = regs->cs;
        e->ec = regs->error_code;
        e->ev = regs->entry_vector;
    }
    else
    {
        printk("Exception log full\n");
        xtf_exlog_dump_log();
        panic("Exception log full\n");
    }

    log_entries++;
}

void xtf_exlog_dump_log(void)
{
    unsigned int i;

    if ( log_entries == 0 )
        printk("No exception log entries\n");
    else
        for ( i = 0; i < ARRAY_SIZE(log) && i < log_entries; ++i )
            printk(" exlog[%02u] %04x:%p vec %u[%04x]\n",
                   i, log[i].cs, _p(log[i].ip), log[i].ev, log[i].ec);
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
