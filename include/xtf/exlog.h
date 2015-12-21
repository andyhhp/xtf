#ifndef XTF_EXLOG_H
#define XTF_EXLOG_H

#include <xtf/types.h>
#include <arch/x86/regs.h>

void xtf_exlog_start(void);
void xtf_exlog_reset(void);
void xtf_exlog_stop(void);

typedef struct exlog_entry
{
    unsigned long ip;
    uint16_t cs, ec, ev;
} exlog_entry_t;

unsigned int xtf_exlog_entries(void);
exlog_entry_t *xtf_exlog_entry(unsigned int idx);

void xtf_exlog_log_exception(struct cpu_regs *regs);

void xtf_exlog_dump_log(void);

#endif /* XTF_EXLOG_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
