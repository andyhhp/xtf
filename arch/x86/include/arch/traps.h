#ifndef XTF_X86_TRAPS_H
#define XTF_X86_TRAPS_H

#include <xtf/compiler.h>
#include <arch/regs.h>
#include <arch/lib.h>
#include <arch/page.h>

#include <xen/xen.h>
#include <xen/arch-x86/hvm/start_info.h>

/*
 * Arch-specific function to initialise the exception entry points, etc.
 */
void arch_init_traps(void);

/*
 * Return the correct %ss/%esp from an exception.  In 32bit if no stack switch
 * occurs, an exception frame doesn't contain this information.
 */
static inline unsigned long cpu_regs_sp(const struct cpu_regs *regs)
{
#ifdef __x86_64__
    return regs->_sp;
#else
    unsigned int cs = read_cs();

    if ( (regs->cs & 3) > (cs & 3) )
        return regs->_sp;

    return _u(regs) + offsetof(struct cpu_regs, _sp);
#endif
}

static inline unsigned int cpu_regs_ss(const struct cpu_regs *regs)
{
#ifdef __x86_64__
    return regs->_ss;
#else
    unsigned int cs = read_cs();

    if ( (regs->cs & 3) > (cs & 3) )
        return regs->_ss;

    return read_ss();
#endif
}

extern uint8_t boot_stack[3 * PAGE_SIZE];
extern uint8_t user_stack[PAGE_SIZE];

extern xen_pv_start_info_t *pv_start_info;
extern xen_pvh_start_info_t *pvh_start_info;

/*
 * Parameters for fine tuning the exec_user_*() behaviour.
 */
extern unsigned long exec_user_cs, exec_user_ss;
extern unsigned long exec_user_efl_and_mask;
extern unsigned long exec_user_efl_or_mask;

#endif /* XTF_X86_TRAPS_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
