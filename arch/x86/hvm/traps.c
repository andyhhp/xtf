#include <xtf/traps.h>
#include <xtf/lib.h>

#include <arch/idt.h>
#include <arch/lib.h>
#include <arch/mm.h>
#include <arch/processor.h>
#include <arch/desc.h>

/* Filled in by hvm/head.S if started via the PVH entrypoint. */
xen_pvh_start_info_t *pvh_start_info;

/* Real entry points */
void entry_DE(void);
void entry_DB(void);
void entry_NMI(void);
void entry_BP(void);
void entry_OF(void);
void entry_BR(void);
void entry_UD(void);
void entry_NM(void);
void entry_DF(void);
void entry_TS(void);
void entry_NP(void);
void entry_SS(void);
void entry_GP(void);
void entry_PF(void);
void entry_MF(void);
void entry_AC(void);
void entry_MC(void);
void entry_XM(void);
void entry_VE(void);
void entry_ret_to_kernel(void);

env_tss tss __aligned(16) =
{
#if defined(__i386__)

    .esp0 = _u(&boot_stack[2 * PAGE_SIZE]),
    .ss0  = __KERN_DS,

    .cr3  = _u(cr3_target),

#elif defined(__x86_64__)

    .rsp0 =   _u(&boot_stack[2 * PAGE_SIZE]),
    .ist[0] = _u(&boot_stack[3 * PAGE_SIZE]),

#endif

    .iopb = X86_TSS_INVALID_IO_BITMAP,
};

static env_tss tss_DF __aligned(16) =
{
#if defined(__i386__)
    .esp  = _u(&boot_stack[3 * PAGE_SIZE]),
    .ss   = __KERN_DS,
    .ds   = __KERN_DS,
    .es   = __KERN_DS,
    .fs   = __KERN_DS,
    .gs   = __KERN_DS,

    .eip  = _u(entry_DF),
    .cs   = __KERN_CS,

    .cr3  = _u(cr3_target),
#endif

    .iopb = X86_TSS_INVALID_IO_BITMAP,
};

static void setup_gate(unsigned int entry, void *addr, unsigned int dpl)
{
    pack_gate(&idt[entry], 14, __KERN_CS, _u(addr), dpl, 0);
}

static void setup_doublefault(void)
{
    if ( IS_DEFINED(CONFIG_32BIT) )
    {
        gdt[GDTE_TSS_DF] = GDTE(_u(&tss_DF), 0x67, 0x89);

        pack_task_gate(&idt[X86_EXC_DF], GDTE_TSS_DF * 8);
    }
    else
        pack_gate(&idt[X86_EXC_DF], 14, __KERN_CS, _u(entry_DF), 0, 1);
}

int xtf_set_idte(unsigned int vector, struct xtf_idte *idte)
{
    pack_gate(&idt[vector], 14, idte->cs, idte->addr, idte->dpl, 0);

    return 0;
}

void arch_init_traps(void)
{
    setup_gate(X86_EXC_DE,  &entry_DE,  0);
    setup_gate(X86_EXC_DB,  &entry_DB,  0);
    setup_gate(X86_EXC_NMI, &entry_NMI, 0);
    setup_gate(X86_EXC_BP,  &entry_BP,  3);
    setup_gate(X86_EXC_OF,  &entry_OF,  3);
    setup_gate(X86_EXC_BR,  &entry_BR,  0);
    setup_gate(X86_EXC_UD,  &entry_UD,  0);
    setup_gate(X86_EXC_NM,  &entry_NM,  0);
    setup_doublefault();
    setup_gate(X86_EXC_TS,  &entry_TS,  0);
    setup_gate(X86_EXC_NP,  &entry_NP,  0);
    setup_gate(X86_EXC_SS,  &entry_SS,  0);
    setup_gate(X86_EXC_GP,  &entry_GP,  0);
    setup_gate(X86_EXC_PF,  &entry_PF,  0);
    setup_gate(X86_EXC_MF,  &entry_MF,  0);
    setup_gate(X86_EXC_AC,  &entry_AC,  0);
    setup_gate(X86_EXC_MC,  &entry_MC,  0);
    setup_gate(X86_EXC_XM,  &entry_XM,  0);
    setup_gate(X86_EXC_VE,  &entry_VE,  0);

    setup_gate(X86_VEC_RET2KERN, &entry_ret_to_kernel, 3);

    lidt(&idt_ptr);

    gdt[GDTE_TSS] = GDTE(_u(&tss), 0x67, 0x89);
    ltr(GDTE_TSS * 8);

    /*
     * If we haven't applied blanket PAGE_USER mappings, remap the structures
     * which specifically want to be user.
     */
    if ( !test_wants_user_mappings )
    {
        unsigned long gfn = virt_to_gfn(user_stack);

        if ( gfn >= ARRAY_SIZE(l1_identmap) )
            panic("user_stack[] outside of l1_identmap[]\n");

        l1_identmap[gfn] |= _PAGE_USER;

        extern const char __start_user_text[], __end_user_text[];
        unsigned long end = virt_to_gfn(__end_user_text);

        if ( gfn >= ARRAY_SIZE(l1_identmap) )
            panic("__{start,end}_user_text[] outside of l1_identmap[]\n");

        for ( gfn = virt_to_gfn(__start_user_text); gfn < end; ++gfn )
            l1_identmap[gfn] |= _PAGE_USER;

        write_cr3(_u(cr3_target));
    }
}

void __noreturn arch_crash_hard(void)
{
    /*
     * Clear interrupts and halt.  Xen should catch this condition and shut
     * the VM down.  If that fails, sit in a loop.
     */
    asm volatile("cli;"
                 "1: hlt;"
                 "pause;"
                 "jmp 1b"
                 ::: "memory");
    unreachable();
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
