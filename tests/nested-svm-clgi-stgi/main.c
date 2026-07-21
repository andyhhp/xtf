/**
 * @file tests/nested-svm-clgi-stgi/main.c
 * @ref test-nested-svm-clgi-stgi
 *
 * @page test-nested-svm-clgi-stgi Smoke-test STGI/CLGI in AMD SVM
 *
 * An L1 guest:
 * 1. enables SVM,
 * 2. builds a minimal L2 VMCB that re-uses L1's address space, and
 * 3. prepares a pending virtual hardware interrupt to be taken in L2.
 * 4. uses CLGI/STGI to manage interrupts in L2.
 *
 * L2:
 * 1. tests receiving the pending virtual hardware interrupt
 * 2. increments or decrements %rax to signal
 *    whether the interrupt was taken or not, and
 * 3. signals completion with HLT, which causes a #VMEXIT back to L1.
 *
 * @see tests/nested-svm-clgi-stgi/main.c
 */
#include <nested-svm/setup-l2.h>

const char test_title[] = "Nested SVM CLGI/STGI Smoke Test";

/* The VMCB's Virtual Interrupt Control field for by-field access. */
static struct v_intr_ctrl_fields *intr_ctrl = &vmcb12.v_intr_ctrl.fields;

/**
 * L2 Interrupt Service Routine for Vector 0x30.
 * It is used to prove the interrupt was actually taken.
 */
static void __used l2_isr_0x30(void)
{
    /* Decrement rax before exiting to indicate that an interrupt was taken */
    asm volatile ("dec %rax;hlt");
}

/**
 * Check if the ISR for 0x30 was called by checking if %rax was decremented.
 * L2 should have decremented %rax from 2 to 1 if the interrupt was taken.
 */
static bool l2_isr_0x30_called(void)
{
    return vmcb12.rax == 1; /* L2 should have decremented %rax from 2 to 1 */
}

static bool l2_isr_0x30_not_called(void)
{
    return vmcb12.rax == 3; /* L2 should have incremented %rax from 2 to 3 */
}

static void __used l2_clgi_hlt(void)
{
    asm volatile ("clgi\n"
                  "hlt");
}

static void __used l2_stgi_hlt(void)
{
    asm volatile ("stgi\n"
                  "hlt");
}

/**
 * L2 payload with STI to take pending interrupts. In case of a pending
 * interrupt, it will be taken after the instruction following STI.
 * If interrupts are disabled, the interrupt will not be taken, inc %rax
 * will increment %rax to signal that exit L2 with HLT.
 */
static void __used l2_sti_nop_inc_rax_hlt(void)
{
    /* STI to allow interrupts, then increment %rax */
    asm volatile ("sti\n"
                  "nop\n" /* Ensures STI takes effect before inc %rax */
                  "inc %rax\n" /* Signal that no IRQ or ISR returned  */
                  "hlt"); /* Exit L2 */
}

static void __used l2_clgi_sti_nop_inc_rax_hlt(void)
{
    asm volatile ("clgi\n"
                  "sti\n"
                  "nop\n"
                  "inc %rax\n"
                  "hlt");
}

static void __used l2_stgi_sti_nop_inc_rax_hlt(void)
{
    asm volatile ("stgi\n"
                  "sti\n"
                  "nop\n"
                  "inc %rax\n"
                  "hlt");
}

static void run_l2_probe_exit(const char *file, int line, uint64_t rip,
                              bool virq_pending)
{
    vmcb12.rax = 2; /* Starting value for L2 to inc/decrement before HLT */
    vmcb12.rip = rip;
    vmcb12.rflags &= ~X86_EFLAGS_IF;

    /* Inject a pending virtual hardware interrupt using the VMCB */
    intr_ctrl->vIRQ_pending = virq_pending;

    print_v_intr_ctrl(file, line, &vmcb12, "pre-VMRUN ");
    asm volatile("mov %0, %%rax\n"
                 "vmload %%rax\n"
                 "vmrun %%rax\n"
                 "vmsave %%rax\n"
                 :
                 : "r" (_u(&vmcb12))
                 : "%rax", "memory");
    print_v_intr_ctrl(file, line, &vmcb12, "post-VMRUN");
}

static bool run_l2_expect_exit(const char *file, int line, uint64_t rip,
                               uint64_t expected_exitcode, bool virq_pending)
{
    run_l2_probe_exit(file, line, rip, virq_pending);

    if ( vmcb12.exitcode != expected_exitcode )
    {
        xtf_failure("%s:%d: unexpected L2 exit: 0x%lx (%s), expected 0x%lx (%s)\n",
                    file, line, vmcb12.exitcode,
                    vmexit_reason(vmcb12.exitcode), expected_exitcode,
                    vmexit_reason(expected_exitcode));
        return false;
    }
    return true;
}

static bool expect_xfail_l2_clgi_stgi_intercepts_without_vgif(void)
{
    uint64_t clgi_exitcode, stgi_exitcode;

    printk("L1: XFAIL testing CLGI/STGI intercepts with vGIF disabled\n");

    intr_ctrl->vGIF_enabled = 0;
    intr_ctrl->vGIF = 0;
    vmcb12.intercept_insns_vec_010.fields.clgi = 1;
    run_l2_probe_exit(__FILE__, __LINE__, _u(l2_clgi_hlt), false);
    clgi_exitcode = vmcb12.exitcode;
    vmcb12.intercept_insns_vec_010.fields.clgi = 0;

    intr_ctrl->vGIF_enabled = 0;
    intr_ctrl->vGIF = 0;
    vmcb12.intercept_insns_vec_010.fields.stgi = 1;
    run_l2_probe_exit(__FILE__, __LINE__, _u(l2_stgi_hlt), false);
    stgi_exitcode = vmcb12.exitcode;
    vmcb12.intercept_insns_vec_010.fields.stgi = 0;

    if ( clgi_exitcode == VMEXIT_HLT && stgi_exitcode == VMEXIT_HLT )
    {
        xtf_warning("XFAIL: Xen did not intercept CLGI/STGI when L1 disabled vGIF\n");
        return true;
    }

    if ( clgi_exitcode == VMEXIT_CLGI && stgi_exitcode == VMEXIT_STGI )
    {
        xtf_failure("XPASS: Xen intercepted CLGI/STGI with vGIF disabled; "
                    "update this test to expect the bug fixed\n");
        return false;
    }

    xtf_failure("Fail: unexpected CLGI/STGI exits with vGIF disabled: "
                "CLGI 0x%lx (%s), STGI 0x%lx (%s)\n",
                clgi_exitcode, vmexit_reason(clgi_exitcode),
                stgi_exitcode, vmexit_reason(stgi_exitcode));
    return false;
}

static bool run_l2(const char *file, int line, uint64_t rip)
{
    return run_l2_expect_exit(file, line, rip, VMEXIT_HLT, true);
}

#define FAIL(fmt, ...) { \
    xtf_failure("%s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
    return false; \
}

/**
 * Test that a pending virtual IRQ is not taken when vGIF is disabled.
 */
bool test_virq_without_vgif(void)
{
    printk("L1: testing pending virtual IRQ without vGIF\n");
    intr_ctrl->vGIF_enabled = 0; /* Disable the vGIF feature */

    if ( !run_l2(__FILE__, __LINE__, _u(l2_sti_nop_inc_rax_hlt)) )
        return false;

    if ( intr_ctrl->vIRQ_pending )
        FAIL("L2 should have cleared the pending virtual IRQ\n");
    if ( !l2_isr_0x30_called() )
        FAIL("L2 0x30 isr was not called as expected\n");
    return true;
}

/**
 * Test that a pending virtual IRQ is not taken when vGIF is enabled but
 * interrupts are disabled in L2.
 */
bool test_virq_with_vgif_irqs_disabled(void)
{
    printk("L1: testing pending virtual IRQ with vGIF disabled\n");
    intr_ctrl->vGIF_enabled = 1; /* Enable vGIF */
    intr_ctrl->vGIF = 0; /* disable interrupts */

    /* Starting value for L2 to inc/decrement before HLT */
    if ( !run_l2(__FILE__, __LINE__, _u(l2_sti_nop_inc_rax_hlt)) )
        return false;

    if ( !intr_ctrl->vIRQ_pending )
        FAIL("L2 should have kept the pending virtual IRQ\n");
    if ( l2_isr_0x30_called() )
        FAIL("L2 0x30 isr called when vGIF:0 should have blocked it\n");
    return true;
}

/**
 * Test that a pending virtual IRQ is taken in L2 when vGIF is enabled and
 * interrupts are enabled.
 */
bool test_virq_with_vgif_irqs_enabled(void)
{
    printk("L1: testing pending virtual IRQ with vGIF enabled\n");
    intr_ctrl->vGIF_enabled = 1; /* Enable vGIF */
    intr_ctrl->vGIF = 1; /* enable interrupts */

    /* Starting value for L2 to inc/decrement before HLT */
    if ( !run_l2(__FILE__, __LINE__, _u(l2_sti_nop_inc_rax_hlt)) )
        return false;

    if ( intr_ctrl->vIRQ_pending )
        FAIL("L2 should have cleared the pending virtual IRQ\n");
    if ( !l2_isr_0x30_called() )
        FAIL("L2 0x30 isr was not called as expected\n");
    return true;
}

/**
 * Test that L2 CLGI and STGI are directly interceptible instructions.
 */
bool test_l2_clgi_stgi_intercepts(void)
{
    printk("L1: testing L2 CLGI/STGI intercepts\n");
    intr_ctrl->vGIF_enabled = 1;

    intr_ctrl->vGIF = 1;
    vmcb12.intercept_insns_vec_010.fields.clgi = 1;
    if ( !run_l2_expect_exit(__FILE__, __LINE__, _u(l2_clgi_hlt),
                             VMEXIT_CLGI, false) )
        return false;
    vmcb12.intercept_insns_vec_010.fields.clgi = 0;

    intr_ctrl->vGIF = 0;
    vmcb12.intercept_insns_vec_010.fields.stgi = 1;
    if ( !run_l2_expect_exit(__FILE__, __LINE__, _u(l2_stgi_hlt),
                             VMEXIT_STGI, false) )
        return false;
    vmcb12.intercept_insns_vec_010.fields.stgi = 0;

    return true;
}

/**
 * Test that L2 CLGI clears vGIF and blocks a pending virtual IRQ.
 */
bool test_l2_clgi_blocks_virq(void)
{
    printk("L1: testing L2 CLGI blocks a pending virtual IRQ\n");
    intr_ctrl->vGIF_enabled = 1;
    intr_ctrl->vGIF = 1;

    if ( !run_l2(__FILE__, __LINE__, _u(l2_clgi_sti_nop_inc_rax_hlt)) )
        return false;

    if ( !intr_ctrl->vIRQ_pending )
        FAIL("L2 CLGI should have kept the pending virtual IRQ\n");
    if ( intr_ctrl->vGIF )
        FAIL("L2 CLGI should have cleared vGIF\n");
    if ( !l2_isr_0x30_not_called() )
        FAIL("L2 0x30 isr called when CLGI should have blocked it\n");
    return true;
}

/**
 * Test that L2 STGI sets vGIF and allows a pending virtual IRQ.
 */
bool test_l2_stgi_allows_virq(void)
{
    printk("L1: testing L2 STGI allows a pending virtual IRQ\n");
    intr_ctrl->vGIF_enabled = 1;
    intr_ctrl->vGIF = 0;

    if ( !run_l2(__FILE__, __LINE__, _u(l2_stgi_sti_nop_inc_rax_hlt)) )
        return false;

    if ( intr_ctrl->vIRQ_pending )
        FAIL("L2 STGI should have cleared the pending virtual IRQ\n");
    if ( !intr_ctrl->vGIF )
        FAIL("L2 STGI should have set vGIF\n");
    if ( !l2_isr_0x30_called() )
        FAIL("L2 0x30 isr was not called after STGI\n");
    return true;
}

/**
 * Execute the nested-SVM CLGI/STGI smoke test.
 *
 * L1 enables SVM, prepares a minimal L2 VMCB, enters L2 once with VMRUN
 * and verifies that L2 reports success before exiting with HLT.
 */
void test_main(void)
{
    /* Enable SVM, arm the host-save area and build the L2 VMCB. */

    if ( !svm_l1_enable_svm() )
        return;

    /* Setup the vmcb for the test */
    svm_l2_build_vmcb(&vmcb12, NULL);

    /* Set up L2's IDTR and install the interrupt gate used by the test. */
    setup_l2_idt(&vmcb12, 0x30, l2_isr_0x30);

    /* Set the injected virtual hardware interrupt to vector 0x30 */
    intr_ctrl->vector = 0x30;

    if ( !test_virq_without_vgif() )
        return;

    if ( !test_virq_with_vgif_irqs_disabled() )
        return;

    if ( !test_virq_with_vgif_irqs_enabled() )
        return;

    if ( !test_l2_clgi_stgi_intercepts() )
        return;

    if ( !test_l2_clgi_blocks_virq() )
        return;

    if ( !test_l2_stgi_allows_virq() )
        return;

    if ( !expect_xfail_l2_clgi_stgi_intercepts_without_vgif() )
        return;

    xtf_success(NULL);
}
