#include <xtf/traps.h>
#include <xtf/lib.h>
#include <xtf/hypercall.h>

#include <arch/x86/processor.h>
#include <arch/x86/segment.h>
#include <arch/x86/mm.h>

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

    { 0x20, 3|4, __KERN_CS, (unsigned long)&entry_ret_to_kernel },

    { 0 }, /* Sentinel. */
};

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
    /*
     * Walk the live pagetables and set _PAGE_USER
     *
     * XTF uses a shared user/kernel address space, and _PAGE_USER must be set
     * to permit cpl3 access to the virtual addresses without taking a
     * pagefault.
     *
     * !!! WARNING !!!
     *
     * Because PV guests and Xen share CR4, Xen's setting of CR4.{SMEP,SMAP}
     * interfere with 32bit PV guests.  For now, 32bit PV XTF tests require
     * the hypervisor to be booted with "smep=0 smap=0".
     *
     * TODO - figure out how to work around this restriction while maintaining
     * a shared user/kernel address space for the framework.
     *
     * !!! WARNING !!!
     */
    uint64_t *l3 = _p(start_info->pt_base);
    unsigned long va = 0;

    while ( va < __HYPERVISOR_VIRT_START_PAE )
    {
        unsigned int i3 = l3_table_offset(va);

        if ( !(l3[i3] & _PAGE_PRESENT) )
        {
            va += 1UL << L3_PT_SHIFT;
            continue;
        }

        uint64_t *l2 = maddr_to_virt(pte_to_paddr(l3[i3]));
        unsigned int i2 = l2_table_offset(va);

        if ( !(l2[i2] & _PAGE_PRESENT) )
        {
            va += 1UL << L2_PT_SHIFT;
            continue;
        }

        uint64_t *l1 = maddr_to_virt(pte_to_paddr(l2[i2]));
        unsigned int i1 = l1_table_offset(va);

        if ( !(l1[i1] & _PAGE_PRESENT) )
        {
            va += 1UL << L1_PT_SHIFT;
            continue;
        }

        if ( !(l1[i1] & _PAGE_USER) )
        {
            rc = hypercall_update_va_mapping(_p(va), l1[i1] | _PAGE_USER, 2);
            if ( rc )
                panic("update_va_mapping(%p, 0x%016"PRIx64") failed: %d\n",
                      _p(va), l1[i1] | _PAGE_USER, rc);
        }

        va += 1UL << L1_PT_SHIFT;
    }
#endif
}

void __noreturn arch_crash_hard(void)
{
    /*
     * For both architectures, put the stack pointer into an invalid place and
     * attempt to use it.  Xen should fail to create a bounce frame and crash
     * the domain.
     */
    asm volatile(
#ifdef __i386__
        "mov %0, %%esp;"
#else
        "movabs %0, %%rsp;"
#endif
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
