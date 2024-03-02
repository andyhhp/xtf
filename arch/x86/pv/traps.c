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

/* Filled in by pv/head.S */
xen_pv_start_info_t *pv_start_info = NULL;

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

void entry_SYSCALL(void);
void entry_SYSENTER(void);
void entry_EVTCHN(void);

struct xen_trap_info pv_default_trap_info[] =
{
    { X86_EXC_DE,  0|4, __KERN_CS, _u(entry_DE)  },
    { X86_EXC_DB,  0|4, __KERN_CS, _u(entry_DB)  },
    { X86_EXC_NMI, 0|4, __KERN_CS, _u(entry_NMI) },
    { X86_EXC_BP,  3|4, __KERN_CS, _u(entry_BP)  },
    { X86_EXC_OF,  3|4, __KERN_CS, _u(entry_OF)  },
    { X86_EXC_BR,  0|4, __KERN_CS, _u(entry_BR)  },
    { X86_EXC_UD,  0|4, __KERN_CS, _u(entry_UD)  },
    { X86_EXC_NM,  0|4, __KERN_CS, _u(entry_NM)  },
    { X86_EXC_DF,  0|4, __KERN_CS, _u(entry_DF)  },
    { X86_EXC_TS,  0|4, __KERN_CS, _u(entry_TS)  },
    { X86_EXC_NP,  0|4, __KERN_CS, _u(entry_NP)  },
    { X86_EXC_SS,  0|4, __KERN_CS, _u(entry_SS)  },
    { X86_EXC_GP,  0|4, __KERN_CS, _u(entry_GP)  },
    { X86_EXC_PF,  0|4, __KERN_CS, _u(entry_PF)  },
    { X86_EXC_MF,  0|4, __KERN_CS, _u(entry_MF)  },
    { X86_EXC_AC,  0|4, __KERN_CS, _u(entry_AC)  },
    { X86_EXC_MC,  0|4, __KERN_CS, _u(entry_MC)  },
    { X86_EXC_XM,  0|4, __KERN_CS, _u(entry_XM)  },
    { X86_EXC_VE,  0|4, __KERN_CS, _u(entry_VE)  },

    { X86_VEC_RET2KERN, 3|4, __KERN_CS, _u(entry_ret_to_kernel) },

    { 0, 0, 0, 0 }, /* Sentinel. */
};

int xtf_set_idte(unsigned int vector, const struct xtf_idte *idte)
{
    struct xen_trap_info ti[2] =
    {
        { vector, idte->dpl | 4, idte->cs, idte->addr },
        { 0, 0, 0, 0 }, /* Sentinel. */
    };

    return hypercall_set_trap_table(ti);
}

static int remap_linear(const void *linear, uint64_t flags)
{
    intpte_t nl1e = pte_from_virt(linear, flags);

    return hypercall_update_va_mapping(_u(linear), nl1e, UVMF_INVLPG);
}

static int __maybe_unused remap_linear_range(const void *start, const void *end,
                                             uint64_t flags)
{
    int ret = 0;

    while ( !ret && start < end )
    {
        ret = remap_linear(start, flags);
        start += PAGE_SIZE;
    }

    return ret;
}

static void init_callbacks(void)
{
    /* PV equivalent of `lidt`. */
    int rc = hypercall_set_trap_table(pv_default_trap_info);

    if ( rc )
        panic("Failed to set trap table: %d\n", rc);

    xen_callback_register_t cb[] = {
        {
            .type = CALLBACKTYPE_event,
            .address = INIT_XEN_CALLBACK(__KERN_CS, _u(entry_EVTCHN)),
        },
#ifdef __x86_64__
        {
            .type = CALLBACKTYPE_syscall,
            .flags = CALLBACKF_mask_events,
            .address = INIT_XEN_CALLBACK(__KERN_CS, _u(entry_SYSCALL)),
        },
#endif
        {
            .type = CALLBACKTYPE_syscall32,
            .flags = CALLBACKF_mask_events,
            .address = INIT_XEN_CALLBACK(__KERN_CS, _u(entry_SYSCALL)),
        },
        {
            .type = CALLBACKTYPE_sysenter,
            .flags = CALLBACKF_mask_events,
            .address = INIT_XEN_CALLBACK(__KERN_CS, _u(entry_SYSENTER)),
        },
    };

    for ( unsigned int i = 0; i < ARRAY_SIZE(cb); ++i )
    {
        rc = hypercall_register_callback(&cb[i]);
        if ( rc )
            panic("Failed to register callback[%u]: %d\n", i, rc);
    }
}

void arch_init_traps(void)
{
    int rc;

    init_callbacks();

    /* Register gdt[] with Xen.  Need to map it read-only first. */
    if ( remap_linear(gdt, PF_SYM(AD, P)) )
        panic("Unable to remap gdt[] as read-only\n");

    unsigned long gdt_frames[] = {
        virt_to_mfn(gdt),
    };
    BUILD_BUG_ON(NR_GDT_ENTRIES > (PAGE_SIZE / sizeof(user_desc)));

    rc = hypercall_set_gdt(gdt_frames, NR_GDT_ENTRIES);
    if ( rc )
        panic("Failed to set gdt: %d\n", rc);

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
    /* Remap the structures which specifically want to be user. */
    extern const char __start_user_text[], __end_user_text[];
    extern const char __start_user_data[], __end_user_data[];
    extern const char __start_user_bss[],  __end_user_bss[];

    remap_linear_range(__start_user_text, __end_user_text,
                       PF_SYM(AD, U, RW, P));

    remap_linear_range(__start_user_data, __end_user_data,
                       PF_SYM(AD, U, RW, P));

    remap_linear_range(__start_user_bss, __end_user_bss,
                       PF_SYM(AD, U, RW, P));
#endif

    /* Unmap page at 0 to catch errors with NULL pointers. */
    rc = hypercall_update_va_mapping(0, 0, UVMF_INVLPG);
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
