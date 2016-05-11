#include <xtf/traps.h>
#include <xtf/lib.h>

#include <arch/x86/lib.h>
#include <arch/x86/processor.h>
#include <arch/x86/desc.h>

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

hw_tss tss __aligned(16) =
{
#if defined(__i386__)

    .esp0 = (unsigned long)&boot_stack[2 * PAGE_SIZE],
    .ss0  = __KERN_DS,

#elif defined(__x86_64__)

    .rsp0 = (unsigned long)&boot_stack[2 * PAGE_SIZE],

#endif

    .iopb = X86_TSS_INVALID_IO_BITMAP,
};

void pack_gate32(struct seg_gate32 *gate, unsigned type, uint32_t func,
                 unsigned dpl, unsigned seg)
{
    gate->offset0 = func & 0xffff;
    gate->selector = seg;
    gate->_r0 = 0;
    gate->type = type;
    gate->s = 0;
    gate->dpl = dpl;
    gate->p = 1;
    gate->offset1 = (func >> 16) & 0xffff;
}

void pack_gate64(struct seg_gate64 *gate, unsigned type, uint64_t func,
                 unsigned dpl, unsigned ist, unsigned seg)
{
    gate->offset0 = func & 0xffff;
    gate->selector = seg;
    gate->ist = ist;
    gate->_r0 = 0;
    gate->type = type;
    gate->s = 0;
    gate->dpl = dpl;
    gate->p = 1;
    gate->offset1 = (func >> 16) & 0xffff;
    gate->offset2 = (func >> 32) & 0xffffffffu;
    gate->_r1 = 0;
}

static void setup_gate(unsigned int entry, void *addr, unsigned int dpl)
{
#if defined(__i386__)
    pack_gate32(&idt[entry], 14, (unsigned long)addr, dpl, __KERN_CS);
#elif defined(__x86_64__)
    pack_gate64(&idt[entry], 14, (unsigned long)addr, dpl, 0, __KERN_CS);
#endif
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
    setup_gate(X86_EXC_DF,  &entry_DF,  0);
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

    setup_gate(0x20, &entry_ret_to_kernel, 3);

    asm volatile ("lidt idt_ptr");

    gdt[GDTE_TSS] = (typeof(*gdt))INIT_GDTE((unsigned long)&tss, 0x67, 0x89);
    asm volatile ("ltr %w0" :: "rm" (GDTE_TSS * 8));
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
