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

env_tss tss __aligned(16) = /* lgtm [cpp/short-global-name] */
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

int xtf_set_idte(unsigned int vector, const struct xtf_idte *idte)
{
    pack_intr_gate(&idt[vector], idte->cs, idte->addr, idte->dpl, 0);

    return 0;
}

static void remap_user(unsigned int start_gfn, unsigned int end_gfn)
{
    while ( start_gfn < end_gfn )
        l1_identmap[start_gfn++] |= _PAGE_USER;
}

void arch_init_traps(void)
{
    pack_intr_gate(&idt[X86_EXC_DE],  __KERN_CS, _u(&entry_DE),  0, 0);
    pack_intr_gate(&idt[X86_EXC_DB],  __KERN_CS, _u(&entry_DB),  0, 0);
    pack_intr_gate(&idt[X86_EXC_NMI], __KERN_CS, _u(&entry_NMI), 0, 0);
    pack_intr_gate(&idt[X86_EXC_BP],  __KERN_CS, _u(&entry_BP),  3, 0);
    pack_intr_gate(&idt[X86_EXC_OF],  __KERN_CS, _u(&entry_OF),  3, 0);
    pack_intr_gate(&idt[X86_EXC_BR],  __KERN_CS, _u(&entry_BR),  0, 0);
    pack_intr_gate(&idt[X86_EXC_UD],  __KERN_CS, _u(&entry_UD),  0, 0);
    pack_intr_gate(&idt[X86_EXC_NM],  __KERN_CS, _u(&entry_NM),  0, 0);
    pack_intr_gate(&idt[X86_EXC_TS],  __KERN_CS, _u(&entry_TS),  0, 0);
    pack_intr_gate(&idt[X86_EXC_NP],  __KERN_CS, _u(&entry_NP),  0, 0);
    pack_intr_gate(&idt[X86_EXC_SS],  __KERN_CS, _u(&entry_SS),  0, 0);
    pack_intr_gate(&idt[X86_EXC_GP],  __KERN_CS, _u(&entry_GP),  0, 0);
    pack_intr_gate(&idt[X86_EXC_PF],  __KERN_CS, _u(&entry_PF),  0, 0);
    pack_intr_gate(&idt[X86_EXC_MF],  __KERN_CS, _u(&entry_MF),  0, 0);
    pack_intr_gate(&idt[X86_EXC_AC],  __KERN_CS, _u(&entry_AC),  0, 0);
    pack_intr_gate(&idt[X86_EXC_MC],  __KERN_CS, _u(&entry_MC),  0, 0);
    pack_intr_gate(&idt[X86_EXC_XM],  __KERN_CS, _u(&entry_XM),  0, 0);
    pack_intr_gate(&idt[X86_EXC_VE],  __KERN_CS, _u(&entry_VE),  0, 0);

    /* Handle #DF with a task gate in 32bit, and IST 1 in 64bit. */
    if ( IS_DEFINED(CONFIG_32BIT) )
    {
        pack_tss_desc(&gdt[GDTE_TSS_DF], &tss_DF);
        pack_task_gate(&idt[X86_EXC_DF], GDTE_TSS_DF * 8);
    }
    else
        pack_intr_gate(&idt[X86_EXC_DF], __KERN_CS, _u(&entry_DF), 0, 1);

    pack_intr_gate(&idt[X86_VEC_RET2KERN],
                   __KERN_CS, _u(&entry_ret_to_kernel), 3, 0);

    lidt(&idt_ptr);

    pack_tss_desc(&gdt[GDTE_TSS], &tss);
    ltr(GDTE_TSS * 8);

    /*
     * If we haven't applied blanket PAGE_USER mappings, remap the structures
     * which specifically want to be user.
     */
    if ( !test_wants_user_mappings )
    {
        extern const char __start_user_text[], __end_user_text[];
        extern const char __start_user_data[], __end_user_data[];
        extern const char __start_user_bss[],  __end_user_bss[];

        remap_user(virt_to_gfn(__start_user_text),
                   virt_to_gfn(__end_user_text));

        remap_user(virt_to_gfn(__start_user_data),
                   virt_to_gfn(__end_user_data));

        remap_user(virt_to_gfn(__start_user_bss),
                   virt_to_gfn(__end_user_bss));

        write_cr3(_u(cr3_target));
    }
}

void __noreturn arch_crash_hard(void)
{
    outb(0, 0xf4);

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
