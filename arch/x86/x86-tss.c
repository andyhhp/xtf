/**
 * @file arch/x86/x86-tss.c
 *
 * %x86 Task State Segment helper routines.
 */
#include <xtf/console.h>

#include <arch/x86-tss.h>

void dump_x86_tss32(const struct x86_tss32 *t)
{
    printk("Dump x86_tss32 at %p {\n"
           "  link %04x\n"
           "  r0 %04x:%08x\n"
           "  r1 %04x:%08x\n"
           "  r2 %04x:%08x\n"
           "  cr3 %08x\n"
           "  eip %08x, eflags %08x\n"
           "  eax %08x, ecx %08x\n"
           "  edx %08x, ebx %08x\n"
           "  esp %08x, ebp %08x\n"
           "  esi %08x, edi %08x\n"
           "  es %04x, cs %04x, ss %04x\n"
           "  ds %04x, fs %04x, gs %04x\n"
           "  ldtr %04x, trace %u, iobp %04x\n"
           "}\n",
           t,
           t->link,
           t->ss0, t->esp0,
           t->ss1, t->esp1,
           t->ss2, t->esp2,
           t->cr3,
           t->eip, t->eflags,
           t->eax, t->ecx,
           t->edx, t->ebx,
           t->esp, t->ebp,
           t->esi, t->edi,
           t->es, t->cs, t->ss,
           t->ds, t->fs, t->gs,
           t->ldtr, t->trace, t->iopb);
}

void dump_x86_tss64(const struct x86_tss64 *t)
{
    printk("Dump x86_tss64 at %p {\n"
           "  rsp0 %016"PRIx64", rsp1 %016"PRIx64"\n"
           "  rsp2 %016"PRIx64", ist1 %016"PRIx64"\n"
           "  ist2 %016"PRIx64", ist3 %016"PRIx64"\n"
           "  ist4 %016"PRIx64", ist5 %016"PRIx64"\n"
           "  ist6 %016"PRIx64", ist7 %016"PRIx64"\n"
           "  trace %u, iobp %04x\n"
           "}\n",
           t,
           t->rsp0, t->rsp1,
           t->rsp2, t->ist[0],
           t->ist[1], t->ist[2],
           t->ist[3], t->ist[4],
           t->ist[5], t->ist[6],
           t->trace, t->iopb);
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
