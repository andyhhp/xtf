/**
 * @file arch/arm/traps.c
 *
 * Arm trap handlers.
 */
#include <xtf/hypercall.h>
#include <arch/regs.h>
#include <arch/system.h>

#ifdef CONFIG_ARM_64
static void show_registers64(struct cpu_regs *regs)
{
    printk(" PC: 0x%016lx\n", regs->pc);
    printk(" LR: 0x%016lx\n", regs->lr);
    printk(" CPSR: 0x%016lx\n", regs->cpsr);
    printk(" SP: 0x%016lx\n", regs->sp);
    printk(" X0: 0x%016lx\t X1: 0x%016lx\tX2: 0x%016lx\n",
           regs->x0, regs->x1, regs->x2);
    printk(" X3: 0x%016lx\t X4: 0x%016lx\tX5: 0x%016lx\n",
           regs->x3, regs->x4, regs->x5);
    printk(" X6: 0x%016lx\t X7: 0x%016lx\tX8: 0x%016lx\n",
           regs->x6, regs->x7, regs->x8);
    printk(" X9: 0x%016lx\tX10: 0x%016lx\tX11: 0x%016lx\n",
           regs->x9, regs->x10, regs->x11);
    printk("X12: 0x%016lx\tX13: 0x%016lx\tX14: 0x%016lx\n",
           regs->x12, regs->x13, regs->x14);
    printk("X15: 0x%016lx\tX16: 0x%016lx\tX17: 0x%016lx\n",
           regs->x15, regs->x16, regs->x17);
    printk("X18: 0x%016lx\tX19: 0x%016lx\tX20: 0x%016lx\n",
           regs->x18, regs->x19, regs->x20);
    printk("X21: 0x%016lx\tX22: 0x%016lx\tX23: 0x%016lx\n",
           regs->x21, regs->x22, regs->x23);
    printk("X24: 0x%016lx\tX25: 0x%016lx\tX26: 0x%016lx\n",
           regs->x24, regs->x25, regs->x26);
    printk("X27: 0x%016lx\tX28: 0x%016lx\tFP: 0x%016lx\n",
           regs->x27, regs->x28, regs->fp);
}
#else
static void show_registers32(struct cpu_regs *regs)
{
    printk(" PC: 0x%08x\n", regs->pc);
    printk(" CPSR: 0x%08x\n", regs->cpsr);
    printk(" R0: 0x%08x\t R1: 0x%08x\tR2: 0x%08x\n",
           regs->r0, regs->r1, regs->r2);
    printk(" R3: 0x%08x\t R4: 0x%08x\tR5: 0x%08x\n",
           regs->r3, regs->r4, regs->r5);
    printk(" R6: 0x%08x\t R7: 0x%08x\tR8: 0x%08x\n",
           regs->r6, regs->r7, regs->r8);
    printk(" R9: 0x%08x\tR10: 0x%08x\tR11: 0x%08x\n",
           regs->r9, regs->r10, regs->fp);
    printk("R12: 0x%08x\n", regs->r12);
}
#endif

static void show_registers(struct cpu_regs *regs)
{
#ifdef CONFIG_ARM_64
    show_registers64(regs);
#else
    show_registers32(regs);
#endif
}

/*
 * Impossible case in the exception vector.
 */
void do_bad_mode(struct cpu_regs *regs)
{
    printk("---Bad mode detected---\n");
    local_irq_disable();
    panic("Bad mode\n");
}

/*
 * Synchronous exception received.
 */
void do_trap_sync(struct cpu_regs *regs)
{
    printk("---Trap sync---\n");
    show_registers(regs);
    panic("Trap sync\n");
}

/*
 * IRQ received.
 */
void do_trap_irq(struct cpu_regs *regs)
{
    UNIMPLEMENTED();
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
