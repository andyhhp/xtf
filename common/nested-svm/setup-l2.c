#include <nested-svm/setup-l2.h>

/* AMD MSRs. */

/* The VMRUN host-save area */
#define MSR_VM_HSAVE_PA 0xc0010117U

/*
 * The L2 VMCB lives here. VMRUN auto-saves and restores the bulk of
 * L1's state via the host-save area pointed to by MSR_VM_HSAVE_PA.
 */
struct vmcb vmcb12 __page_aligned_bss;

/* Backing store for the VMRUN host-save area (MSR_VM_HSAVE_PA). */
uint8_t hsave[PAGE_SIZE] __page_aligned_bss;

/* Stack used by L2.  Two pages of backing store. */
uint8_t l2_stack[2 * PAGE_SIZE] __page_aligned_bss;

/* Private IDT used by L2 guests in nested-SVM tests. */
static env_gate l2_idt[256] __page_aligned_bss;

/* Build a segment descriptor for the VMCB from a user descriptor. */
static uint16_t user_desc_vmcb_attr(const user_desc *desc)
{
    return desc->type |
        (desc->s << 4) |
        (desc->dpl << 5) |
        (desc->p << 7) |
        (desc->limit1 << 8) |
        (desc->avl << 12) |
        (desc->l << 13) |
        (desc->d << 14) |
        (desc->g << 15);
}

/* Check if a selector is null. */
static bool selector_is_null(uint16_t sel)
{
    return !(sel & ~(X86_SEL_TI | X86_SEL_RPL_MASK));
}

/* Mark a segment as unusable in the VMCB. */
static void vmcb_set_seg_unusable(struct vmcb_seg *seg, uint16_t sel)
{
    seg->sel = sel;
    seg->attr = 0;
    seg->limit = 0;
    seg->base = 0;
}

/* Build a segment descriptor for the VMCB from a user descriptor. */
static void vmcb_set_seg_desc(struct vmcb_seg *seg, const user_desc *gdt,
                              uint16_t gdt_limit, uint16_t sel)
{
    uint16_t sel_offset = sel & ~(X86_SEL_TI | X86_SEL_RPL_MASK);
    unsigned int gdt_desc_bytes = sizeof(*gdt);
    const user_desc *desc;

    if ( selector_is_null(sel) )
    {
        vmcb_set_seg_unusable(seg, sel);
        return;
    }

    if ( (sel & X86_SEL_TI) ||
         (sel_offset + gdt_desc_bytes - 1 > gdt_limit) )
    {
        vmcb_set_seg_unusable(seg, 0);
        return;
    }

    desc = (const user_desc *)((const char *)gdt + sel_offset);

    if ( !desc->s )
        gdt_desc_bytes *= 2;

    if ( sel_offset + gdt_desc_bytes - 1 > gdt_limit )
    {
        vmcb_set_seg_unusable(seg, 0);
        return;
    }

    seg->sel = sel;
    seg->attr = user_desc_vmcb_attr(desc);
    seg->limit = user_desc_limit(desc);
    seg->base = user_desc_base(desc);
}

/* set or clear EFER.SVME and return the original EFER value */
uint64_t update_efer_svme(bool enable)
{
    uint64_t efer = rdmsr(MSR_EFER);
    uint64_t new_efer = enable ? (efer | EFER_SVME) : (efer & ~EFER_SVME);

    wrmsr(MSR_EFER, new_efer);
    return efer;
}

/* Enable SVM in L1 and program the host-save area used by VMRUN. */
bool svm_l1_enable_svm(void)
{
    if ( !cpu_has_svm ) {
        xtf_skip("Skip: SVM not available\n");
        return false;
    }

    update_efer_svme(true);
    wrmsr(MSR_VM_HSAVE_PA, _u(hsave));
    return true;
}

/* Build a minimal long-mode L2 VMCB that reuses the current L1 environment. */
void svm_l2_build_vmcb(struct vmcb *vmcb, const struct svm_l2_config *cfg)
{
    struct svm_l2_config default_l2 = {
        .efer = rdmsr(MSR_EFER),
        .intercept_insns_vec_00c.fields.hlt = 1,
        .intercept_insns_vec_010.fields.vmrun = 1,
        .rsp = _u(&l2_stack[sizeof(l2_stack)]),
    };
    desc_ptr gdt_desc, idt_desc;
    const user_desc *gdt;

    memset(vmcb, 0, sizeof(*vmcb));

    if ( !cfg )
        cfg = &default_l2;
    vmcb->intercept_insns_vec_00c = cfg->intercept_insns_vec_00c;
    vmcb->intercept_insns_vec_010 = cfg->intercept_insns_vec_010;
    vmcb->intercept_insns_vec_014 = cfg->intercept_insns_vec_014;
    vmcb->asid = cfg->asid ? cfg->asid : 1;

    vmcb->cr0    = read_cr0();
    vmcb->cr3    = read_cr3();
    vmcb->cr4    = read_cr4();
    vmcb->efer   = cfg->efer;
    vmcb->rflags = read_flags();

    vmcb->rsp = cfg->rsp;
    vmcb->rip = cfg->rip;

    sgdt(&gdt_desc);
    sidt(&idt_desc);
    vmcb->v_intr_ctrl.fields.vIRQ_prio = 2; /* vIRQ priority */
    vmcb->gdtr.base  = gdt_desc.base;
    vmcb->gdtr.limit = gdt_desc.limit;
    vmcb->idtr.base  = idt_desc.base;
    vmcb->idtr.limit = idt_desc.limit;
    gdt = (const user_desc *)gdt_desc.base;

    vmcb_set_seg_desc(&vmcb->ldtr, gdt, gdt_desc.limit, sldt());
    vmcb_set_seg_desc(&vmcb->tr, gdt, gdt_desc.limit, str());

    vmcb->cs.sel = __KERN_CS;
    vmcb->cs.attr = 0xa9b;
    vmcb->cs.limit = ~0u;

    vmcb->ds.sel = __USER_DS;
    vmcb->ds.attr = 0xcf3;
    vmcb->ds.limit = ~0u;
    vmcb->es = vmcb->fs = vmcb->gs = vmcb->ds;

    vmcb->ss.sel = __KERN_DS;
    vmcb->ss.attr = 0;
    vmcb->ss.limit = 0;
}

/* Build an L2 IDT with a single interrupt gate for the supplied vector. */
void setup_l2_idt(struct vmcb *vmcb, unsigned int vector,
                  void (*handler)(void))
{
    pack_intr_gate(&l2_idt[vector], __KERN_CS, _u(handler), 0, 0);

    vmcb->idtr.base = _u(l2_idt);
    vmcb->idtr.limit = sizeof(l2_idt) - 1;
}

void print_efer(const char *prefix, uint64_t efer)
{
    printk("%s: EFER: 0x%lx, set:", prefix, efer);
    if (efer & (1u << 0))  printk(" SCE");
    if (efer & (1u << 8))  printk(" LME");
    if (efer & (1u << 10)) printk(" LMA");
    if (efer & (1u << 11)) printk(" NXE");
    if (efer & (1u << 12)) printk(" SVME");
    printk("\n");
}
