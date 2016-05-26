#ifndef XTF_X86_XTF_H
#define XTF_X86_XTF_H

#include <arch/apic.h>
#include <arch/cpuid.h>
#include <arch/decode.h>
#include <arch/exinfo.h>
#include <arch/hpet.h>
#include <arch/idt.h>
#include <arch/io-apic.h>
#include <arch/lib.h>
#include <arch/mm.h>
#include <arch/msr.h>
#include <arch/pagetable.h>
#include <arch/symbolic-const.h>
#include <arch/test.h>
#include <arch/tsx.h>
#include <arch/x86-dbg-reg.h>

extern char _start[], _end[];

/*** Misc helpers which are library code, but really want to be inline. ***/

/**
 * Helper to update a live LDT/GDT entry.
 */
static inline void update_desc(user_desc *ptr, const user_desc new)
{
    if ( IS_DEFINED(CONFIG_HVM) )
    {
        *ptr = new;

        /*
         * Prevent the compiler reordering later operations which refer to the
         * descriptor which has been updated.
         */
        barrier();
    }
    else
    {
        int rc = hypercall_update_descriptor(virt_to_maddr(ptr), new);
        if ( rc )
            panic("Update descriptor failed: %d\n", rc);
    }
}

#endif /* XTF_X86_XTF_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
