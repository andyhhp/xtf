#ifndef XTF_EXTABLE_H
#define XTF_EXTABLE_H

#include <xtf/types.h>
#include <xtf/asm_macros.h>

#ifdef __ASSEMBLY__

/* Exception Table entry for asm code. */
#define _ASM_EXTABLE(fault, fixup)              \
    .pushsection .ex_table, "a";                \
    _WORD fault, fixup;                         \
    .popsection

#else

/* Exception Table entry for C inline assembly. */
#define _ASM_EXTABLE(fault, fixup)              \
    ".pushsection .ex_table, \"a\";\n"          \
    _WORD #fault ", " #fixup ";\n"              \
    ".popsection;\n"

/*
 * Sort the exception table.  Required to be called once on boot to make
 * searching efficient.
 */
void sort_extable(void);

/*
 * Search the exception table to see whether an entry has been registered for
 * the provided fault address.  If so, returns the fixup address.  If not,
 * returns zero.
 */
unsigned long search_extable(unsigned long fault_addr);

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
