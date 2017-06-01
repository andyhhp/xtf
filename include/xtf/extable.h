/**
 * @file include/xtf/extable.h
 *
 * Exception table support.
 *
 * Allows code to tag an instruction which might fault, and where to jump to
 * in order to recover.  For more complicated recovery, a cusom handler
 * handler can be registerd.
 */
#ifndef XTF_EXTABLE_H
#define XTF_EXTABLE_H

#include <xtf/types.h>
#include <xtf/asm_macros.h>
#include <xtf/macro_magic.h>

#ifdef __ASSEMBLY__

/**
 * Create an exception table entry with custom handler.
 * @param fault Faulting address.
 * @param fixup Fixup address.
 * @param handler Handler to call.
 */
#define _ASM_EXTABLE_HANDLER(fault, fixup, handler) \
    .pushsection .ex_table, "a";                    \
    _WORD fault, fixup, handler;                    \
    .popsection

#else

/**
 * Create an exception table entry with custom handler.
 * @param fault Faulting address.
 * @param fixup Fixup address.
 * @param handler Handler to call.
 */
#define _ASM_EXTABLE_HANDLER(fault, fixup, handler)             \
    ".pushsection .ex_table, \"a\";\n"                          \
    _WORD STR(fault) ", " STR(fixup) ", " STR(handler) ";\n"    \
    ".popsection;\n"

#endif

/**
 * Create an exception table entry.
 * @param fault Faulting address.
 * @param fixup Fixup address.
 */
#define _ASM_EXTABLE(fault, fixup) _ASM_EXTABLE_HANDLER(fault, fixup, 0)

/**
 * Create an exception table entry, whitelisting a trap as being ok at @param
 * loc.  (ab)uses the fault fixup logic to fixup to its current position.
 */
#define _ASM_TRAP_OK(loc) _ASM_EXTABLE(loc, loc)

#ifndef __ASSEMBLY__

struct cpu_regs;

/** Exception table entry. */
struct extable_entry
{
    unsigned long fault; /**< Faulting address. */
    unsigned long fixup; /**< Fixup address. */

    /**
     * Optional custom handler.
     *
     * If provided, the handler is responsible for altering regs->ip to avoid
     * the fault.
     *
     * @param regs Register state from the fault.
     * @param ex extable_entry for the fault.
     * @return true if the fault was successfully handled.  false otherwise.
     */
    bool (*handler)(struct cpu_regs *regs,
                    const struct extable_entry *ex);
};

/**
 * Sort the exception table.  Required to be called once on boot to make
 * searching efficient.
 */
void sort_extable(void);

/**
 * Search the exception table to find the entry associated with a specific
 * faulting address.
 * @param addr Faulting address.
 * @returns Appropriate extable_entry, or NULL if no entry.
 */
const struct extable_entry *search_extable(unsigned long addr);

#include <arch/extable.h>

#endif /* __ASSEMBLY__ */

#endif /* XTF_EXTABLE_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
