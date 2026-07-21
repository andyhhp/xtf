/**
 * @file tests/nested-svm-run/main.c
 * @ref test-nested-svm-run
 * @page test-nested-svm-run Nested SVM VMRUN Smoke Test
 *
 * An L1 guest:
 * 1. enables SVM,
 * 2. builds a minimal L2 VMCB that re-uses L1's address space, and
 * 3. uses VMRUN to enter an L2 callback.
 *
 * L2:
 * 1. increments %rax
 * 2. signals completion with HLT, which causes a #VMEXIT back to L1.
 * 3. checks that HLT with L2 IF clear doesn't power off L1 when L1 has the
 *    INTR intercept armed.
 *
 * @see tests/nested-svm-run/main.c
 */
#include <nested-svm/setup-l2.h>

const char test_title[] = "Nested SVM VMRUN Smoke Test";

/**
 * Run a minimal L2 payload and report success back to L1.
 */
static void __used l2_entry(void)
{
    asm volatile ("inc %rax;hlt"); /* Signal success by incrementing %rax */
}

static void __used l2_cli_hlt(void)
{
    asm volatile ("cli;hlt");
}

static void run_l2(void)
{
    asm volatile("mov %0, %%rax\n"
                 "vmload %%rax\n"
                 "vmrun %%rax\n"
                 "vmsave %%rax\n"
                 :
                 : "r" (_u(&vmcb12))
                 : "%rax", "memory");
}

/*
 * Test that L2 CLI;HLT with L1 INTR intercept armed does not power off L1.
 * This is a known Xen bug that is expected to be fixed in the future:
 *
 * Issue to be fixed: L1 killed if L2 halts while not intercepted:
 * (XEN) arch/x86/hvm/hvm.c:1735:d13v0 All CPUs offline -- powering off.
 * (XEN) vcpu_runstate_change: d13 has no online vcpus!
 *
 * Possible cause:
 * This code may be wrong since L1 called vmrun with the IF flag
 * set and the INTR intercept enabled. It could come from this
 * check which only checks eflags from L2:
 *
 * >  * If we halt with interrupts disabled, that's a pretty sure sign that we
 * >  * want to shut down. In a real processor, NMIs are the only way to break
 * >  * out of this.
 * >
 * > if ( unlikely(!(eflags & X86_EFLAGS_IF)) )
 * >     return hvm_vcpu_down(curr);
 */
static bool expect_xfail_l2_cli_hlt_with_l1_intr_intercept(void)
{
    unsigned long l1_rflags = read_flags();

    printk("L1: XFAIL testing L2 CLI;HLT with L1 INTR intercept armed\n");

    svm_l2_build_vmcb(&vmcb12, NULL);
    vmcb12.intercept_insns_vec_00c.fields.hlt = 0;
    vmcb12.intercept_insns_vec_00c.fields.intr = 1;
    vmcb12.rflags |= X86_EFLAGS_IF;
    vmcb12.rip = _u(l2_cli_hlt);

    write_flags(l1_rflags | X86_EFLAGS_IF);
    if ( !(read_flags() & X86_EFLAGS_IF) )
    {
        xtf_error("L1 IF is clear before VMRUN\n");
        write_flags(l1_rflags);
        return false;
    }

    xtf_warning("XFAIL: broken Xen powers off L1 after L2 CLI;HLT with "
                "HLT intercept clear and INTR intercept set\n");
    printk("L1: entering L2 via VMRUN with HLT intercept clear, INTR intercept set\n");
    run_l2();
    write_flags(l1_rflags);
    printk("L1: returned from L2 (exit 0x%lx %s)\n",
           vmcb12.exitcode, vmexit_reason(vmcb12.exitcode));

    if ( vmcb12.exitcode == VMEXIT_INTR || vmcb12.exitcode == VMEXIT_VINTR )
    {
        xtf_failure("XPASS: L2 CLI;HLT exited to L1 via %s; "
                    "update this test to expect the bug fixed\n",
                    vmexit_reason(vmcb12.exitcode));
        return false;
    }

    xtf_warning("XFAIL: L2 CLI;HLT did not exit to L1 via the INTR intercept\n");
    return true;
}

/**
 * Execute the nested-SVM VMRUN smoke test.
 *
 * L1 enables SVM, prepares a minimal L2 VMCB, enters L2 once with VMRUN
 * and verifies that L2 reports success before exiting with HLT.
 */
void test_main(void)
{
    /* Enable SVM, arm the host-save area and build the L2 VMCB. */
    if (!svm_l1_enable_svm())
        return;

    svm_l2_build_vmcb(&vmcb12, NULL);

    /* Set the L2 entry point to this test's l2_entry function. */
    vmcb12.rip = _u(l2_entry);

    print_efer("VMCB12", vmcb12.efer);
    printk("L1: entering L2 via VMRUN\n");
    run_l2();
    printk("L1: returned from L2 (rax 0x%lx)\n", vmcb12.rax);

    if ( vmcb12.exitcode != VMEXIT_HLT )
    {
        printk("Exit reason: %s\n", vmexit_reason(vmcb12.exitcode));
        return xtf_failure("unexpected L2 exit: 0x%lx\n", vmcb12.exitcode);
    }

    if ( vmcb12.rax != 1 ) /* L2 should have incremented %rax from 0 to 1 */
        return xtf_failure("unexpected L2 %%rax: 0x%lx\n", vmcb12.rax);

    if ( !expect_xfail_l2_cli_hlt_with_l1_intr_intercept() )
        return;

    xtf_success(NULL);
}
