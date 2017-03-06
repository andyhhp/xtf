#include <xtf/traps.h>
#include <xtf/lib.h>
#include <xtf/hypercall.h>
#include <xtf/test.h>

#include <arch/idt.h>
#include <arch/lib.h>
#include <arch/processor.h>
#include <arch/segment.h>
#include <arch/pagetable.h>
#include <arch/symbolic-const.h>

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

struct xen_trap_info pv_default_trap_info[] =
{
    { X86_EXC_DE,  0|4, __KERN_CS, (unsigned long)&entry_DE  },
    { X86_EXC_DB,  0|4, __KERN_CS, (unsigned long)&entry_DB  },
    { X86_EXC_NMI, 0|4, __KERN_CS, (unsigned long)&entry_NMI },
    { X86_EXC_BP,  3|4, __KERN_CS, (unsigned long)&entry_BP  },
    { X86_EXC_OF,  3|4, __KERN_CS, (unsigned long)&entry_OF  },
    { X86_EXC_BR,  0|4, __KERN_CS, (unsigned long)&entry_BR  },
    { X86_EXC_UD,  0|4, __KERN_CS, (unsigned long)&entry_UD  },
    { X86_EXC_NM,  0|4, __KERN_CS, (unsigned long)&entry_NM  },
    { X86_EXC_DF,  0|4, __KERN_CS, (unsigned long)&entry_DF  },
    { X86_EXC_TS,  0|4, __KERN_CS, (unsigned long)&entry_TS  },
    { X86_EXC_NP,  0|4, __KERN_CS, (unsigned long)&entry_NP  },
    { X86_EXC_SS,  0|4, __KERN_CS, (unsigned long)&entry_SS  },
    { X86_EXC_GP,  0|4, __KERN_CS, (unsigned long)&entry_GP  },
    { X86_EXC_PF,  0|4, __KERN_CS, (unsigned long)&entry_PF  },
    { X86_EXC_MF,  0|4, __KERN_CS, (unsigned long)&entry_MF  },
    { X86_EXC_AC,  0|4, __KERN_CS, (unsigned long)&entry_AC  },
    { X86_EXC_MC,  0|4, __KERN_CS, (unsigned long)&entry_MC  },
    { X86_EXC_XM,  0|4, __KERN_CS, (unsigned long)&entry_XM  },
    { X86_EXC_VE,  0|4, __KERN_CS, (unsigned long)&entry_VE  },

    { X86_VEC_RET2KERN, 3|4, __KERN_CS, (unsigned long)&entry_ret_to_kernel },

    { 0, 0, 0, 0 }, /* Sentinel. */
};

int xtf_set_idte(unsigned int vector, struct xtf_idte *idte)
{
    struct xen_trap_info ti[2] =
    {
        { vector, idte->dpl | 4, idte->cs, idte->addr },
        { 0, 0, 0, 0 }, /* Sentinel. */
    };

    return hypercall_set_trap_table(ti);
}

#ifdef __i386__
static bool __used ex_pf_user(struct cpu_regs *regs,
                              const struct extable_entry *ex)
{
    if ( regs->entry_vector == X86_EXC_PF && read_cr2() == 0xfff )
    {
        regs->ax = true;
        regs->ip = ex->fixup;

        return true;
    }

    return false;
}
#endif

void arch_init_traps(void)
{
    /* PV equivalent of `lidt`. */
    int rc = hypercall_set_trap_table(pv_default_trap_info);

    if ( rc )
        panic("Failed to set trap table: %d\n", rc);

    /* PV equivalent of setting tss.{esp0,ss0}. */
    rc = hypercall_stack_switch(__KERN_DS, &boot_stack[2 * PAGE_SIZE]);
    if ( rc )
        panic("Failed to set kernel stack: %d\n", rc);

    write_ds(__USER_DS);
    write_es(__USER_DS);
    write_fs(__USER_DS);
    write_gs(__USER_DS);

#ifdef __x86_64__
    /*
     * Set the user pagetables (only applicable to 64bit PV).
     *
     * XTF uses a shared user/kernel address space, so register the kernel
     * %cr3 as the user %cr3.
     */
    mmuext_op_t ext =
    {
        .cmd = MMUEXT_NEW_USER_BASEPTR,
        .arg1.mfn = read_cr3() >> PAGE_SHIFT,
    };

    rc = hypercall_mmuext_op(&ext, 1, NULL, DOMID_SELF);
    if ( rc )
        panic("Failed to set user %%cr3: %d\n", rc);

#elif defined(__i386__)
    /* Remap user areas with _PAGE_USER. */
    intpte_t nl1e = pte_from_virt(user_stack, PF_SYM(AD, U, RW, P));

    if ( hypercall_update_va_mapping(user_stack, nl1e, UVMF_INVLPG) )
        panic("Unable to remap user_stack with _PAGE_USER\n");

    extern const char __start_user_text[], __end_user_text[];
    unsigned long va = (unsigned long)__start_user_text;
    while ( va < (unsigned long)__end_user_text )
    {
        nl1e = pte_from_virt(_p(va), PF_SYM(AD, U, RW, P));

        if ( hypercall_update_va_mapping(_p(va), nl1e, UVMF_INVLPG) )
            panic("Unable to remap user_text with _PAGE_USER\n");

        va += PAGE_SIZE;
    }
#endif

    /* Unmap page at 0 to catch errors with NULL pointers. */
    rc = hypercall_update_va_mapping(NULL, 0, UVMF_INVLPG);
    if ( rc )
        panic("Failed to unmap page at NULL: %d\n", rc);
}

void __noreturn arch_crash_hard(void)
{
    /*
     * For both architectures, put the stack pointer into an invalid place and
     * attempt to use it.  Xen should fail to create a bounce frame and crash
     * the domain.
     */
    asm volatile(
        "mov %0, %%" _ASM_SP ";"
        "pushf"
        ::
#ifdef __i386__
         "i" (0xfbadc0deUL)         /* 32bit: In the read-only M2P mapping. */
#else
         "i" (0x800000000badc0deUL) /* 64bit: Non-canonical region. */
#endif
        : "memory" );

    /*
     * Attempt to crash failed.  Give up and sit in a loop.
     */
    asm volatile("1: hlt;"
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
