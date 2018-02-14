/**
 * @file tests/nmi-taskswitch-priv/main.c
 * @ref test-nmi-taskswitch-priv
 *
 * @page test-nmi-taskswitch-priv NMI Taskswitch with increasing privilege.
 *
 * 32bit versions of Windows use a Task Gate for handling @#NMI, and use NMI
 * IPIs between vCPUs in some circumstances.
 *
 * A regression was introduced during the Xen 4.9 development cycle which
 * caused a task switch which changed CPL to corrupt VMCB/VMCS state and
 * result in a vmentry failure.  To make this regression test reliable, it is
 * simplified to a single vCPU using a self-NMI IPI to trigger the task
 * switch.
 *
 * **Notes for people writing reusable x86 tasks:**
 *
 * %x86 Tasks and hardware task switching are basically unused in practice.
 * As a result, there is precious little information online.  The following
 * issues caught me by surprise while developing this code.  Some are now
 * obvious in retrospect, while some are not.
 *
 * - Entering a task doesn't push an exception frame, although an error code
 *   will be pushed if applicable.  To exit from the task using iret, you must
 *   either land under a prepared iret frame with eflags.NT set, or have the
 *   handler construct one itself.
 *
 * - Exiting a task overwrites all GPR state in the TSS, which gets recalled
 *   on the subsequent entry.  For the task to be reusable, the iret to leave
 *   it must be immediately before the entry point.
 *
 * - The state saved on a successful task exit will be the @%eip following the
 *   iret instruction, but the @%esp at the start of the instruction;
 *   **despite the impression by the SDM pseudocode**.  You either need to
 *   land under a prepared frame, or leave 3 slots of room above the
 *   entrypoint to construct a suitable frame.
 *
 * @see tests/nmi-taskswitch-priv/main.c
 */
#include <xtf.h>

#include <arch/apic.h>

const char test_title[] = "Test nmi-taskswitch-priv";

bool test_wants_user_mappings = true;

static uint32_t nmi_stack[PAGE_SIZE / sizeof(uint32_t)] __page_aligned_data =
{
    [ ARRAY_SIZE(nmi_stack) - 8 ] =
    0xdeadbeef,                         /* %eip    */
    0x0000dead,                         /* %cs     */
    X86_EFLAGS_NT | X86_EFLAGS_MBS,     /* eflags  */
    0xdeadbeef,                         /* %esp    */
    0x0000dead,                         /* %ss     */
    0xc2c2c2c2, 0xc2c2c2c2, 0xc2c2c2c2, /* poision */
};

void entry_NMI_task(void);
asm("exit_NMI_task:"
    "iret;"
    "entry_NMI_task:;"

    "push $0;"
    "push $" STR(X86_EXC_NMI) ";"

    "push %es;"
    "push %ds;"

    "push %edi;"
    "push %esi;"
    "push %edx;"
    "push %ecx;"
    "push %eax;"
    "push %ebx;"
    "push %ebp;"

    "push %esp;"
    "call do_exception;"
    "add $1*4, %esp;"

    "pop %ebp;"
    "pop %ebx;"
    "pop %eax;"
    "pop %ecx;"
    "pop %edx;"
    "pop %esi;"
    "pop %edi;"

    "pop %ds;"
    "pop %es;"

    "add $2*4, %esp;" /* entry_vector/error_code. */

    "jmp exit_NMI_task;");


static env_tss nmi_tss __aligned(16) =
{
    .eip    = _u(entry_NMI_task),
    .cs     = __KERN_CS,
    .eflags = X86_EFLAGS_MBS,
    .esp    = _u(&nmi_stack[ARRAY_SIZE(nmi_stack) - 8]),
    .ss     = __KERN_DS,

    .ds     = __USER_DS,
    .es     = __USER_DS,
    .fs     = __USER_DS,
    .gs     = __USER_DS,

    .cr3    = _u(cr3_target),
    .iopb   = X86_TSS_INVALID_IO_BITMAP,
};

static bool unhandled_exception(struct cpu_regs *regs)
{
    if ( regs->entry_vector != X86_EXC_NMI )
        return false;

    unsigned int curr_ts = str();
    if ( curr_ts != GDTE_AVAIL0 * 8 )
        xtf_failure("Fail: Running NMI handler with unexpected %%tr\n"
                    "  Expected %04x, got %04x\n", GDTE_AVAIL0 * 8, curr_ts);

    /* Follow the TSS link pointer to get the interrupted state. */
    env_tss *t = _p(user_desc_base(&gdt[nmi_tss.link >> 3]));

    printk("  NMI at %04x:%p, stack %04x:%p\n",
           t->cs, _p(t->eip), t->ss, _p(t->esp));

    return true;
}

static void __user_text user_inject_nmi(void)
{
    apic_mmio_icr_write(APIC_DEST_SELF | APIC_DM_NMI);
}

void test_main(void)
{
    unsigned int curr_ts;
    int rc = apic_init(APIC_MODE_XAPIC);

    if ( rc )
        return xtf_error("Error: Unable to set up xapic mode: %d\n", rc);

    /*
     * Set up NMI handling to be a task gate.
     */
    xtf_unhandled_exception_hook = unhandled_exception;
    gdt[GDTE_AVAIL0] = (user_desc)INIT_GDTE(_u(&nmi_tss), 0x67, 0x89);
    pack_task_gate(&idt[X86_EXC_NMI], GDTE_AVAIL0 * 8);
    barrier();

    /*
     * Send an NMI from supervisor mode, checking that we task switch back to
     * the expected TSS.
     */
    printk("First self-nmi, from supervisor mode\n");
    apic_mmio_icr_write(APIC_DEST_SELF | APIC_DM_NMI);

    if ( (curr_ts = str()) != (GDTE_TSS * 8) )
        xtf_failure("Fail: Running main task with unexpected %%tr\n"
                    "  Expected %04x, got %04x\n", (GDTE_TSS * 8), curr_ts);

    /*
     * Send an NMI from user mode, and again check that we are in the expected
     * task.
     */
    printk("Second self-nmi, from user mode\n");
    exec_user_void(user_inject_nmi);

    if ( (curr_ts = str()) != (GDTE_TSS * 8) )
        xtf_failure("Fail: Running main task with unexpected %%tr\n"
                    "  Expected %04x, got %04x\n", (GDTE_TSS * 8), curr_ts);

    /*
     * If Xen is still alive, it handled the user=>supervisor task switch
     * properly.
     */
    xtf_success(NULL);
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
