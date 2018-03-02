/**
 * @file tests/xsa-186/main.c
 * @ref test-xsa-186
 *
 * @page test-xsa-186 XSA-186
 *
 * Advisory: [XSA-186](http://xenbits.xen.org/xsa/advisory-186.html)
 *
 * Experimentally, Intel and AMD hardware is happy executing a 64bit
 * instruction stream which crosses the -1 -> 0 virtual boundary, whether the
 * instruction boundary is aligned on the virtual boundary, or is misaligned.
 *
 * For 32bit, Intel processors are happy executing an instruction stream which
 * crosses the -1 -> 0 virtual boundary, while AMD processors raise a
 * segmentation fault.
 *
 * For 16bit code, hardware does not ever truncated @%ip.  @%eip is always
 * used and behaves normally as a 32bit register, including in 16bit protected
 * mode segments, as well as in Real and Unreal mode.
 *
 * The upstream change [0640ffb6](http://xenbits.xen.org/gitweb/
 * ?p=xen.git;a=commitdiff;h=0640ffb67fb92e2561c63b9308c27b71281fdd72) broke
 * this behaviour, and introduced conditions which resulted in x86 emulator
 * state corruption.
 *
 * This test sets up two situations in boundary conditions, and checks that
 * the code gets executed as expected.  In the case where execution doesn't
 * proceed as expected, it is very likely that state corruption has occurred,
 * even if Xen didn't crash immediately.
 *
 * @see tests/xsa-186/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-186 PoC";

bool test_needs_fep = true;

/* Test-local `int` handler. */
void test_int_handler(void);
asm(".align 16;"
    "test_int_handler:"
    "movl $0xc0de, %eax;"
#ifdef __x86_64__
    "rex64 "
#endif
    "iret"
    );

/* Stub instruction buffer. */
asm(".align 16;"
    "insn_stub_start:;"
    _ASM_XEN_FEP
    ".Lstub_fault:;"
    ".byte 0x67; int $" STR(X86_VEC_AVAIL) ";"
    ".Lstub_fixup: ret;"
    "insn_stub_end:;"
    );

static bool ex_fault(struct cpu_regs *regs, const struct extable_entry *ex)
{
    if ( regs->entry_vector == X86_EXC_GP && regs->error_code == 0 )
    {
        extern char _fault[] asm(".Lstub_fault");
        extern char _fixup[] asm(".Lstub_fixup");
        const size_t delta = _fixup - _fault;

        regs->ax = 0xc0de;
        regs->ip += delta;
        return true;
    }

    return false;
}

void test_main(void)
{
    struct xtf_idte idte =
    {
        .addr = _u(test_int_handler),
        .cs = __KERN_CS,
    };

    /* Hook test_int_handler() into the real IDT. */
    xtf_set_idte(X86_VEC_AVAIL, &idte);

    /* Reinstate the mapping at 0 virtual.  It will be needed. */
    if ( CONFIG_PAGING_LEVELS > 0 )
    {
        l1_identmap[0] = pte_from_gfn(0, PF_SYM(AD, RW, P));
        barrier();
    }

    if ( IS_DEFINED(CONFIG_64BIT) )
    {
        static intpte_t nl1t[L1_PT_ENTRIES] __page_aligned_bss;
        static intpte_t nl2t[L2_PT_ENTRIES] __page_aligned_bss;

        /*
         * The manuals say we can't `iretq` to a compat mode code segment with
         * a NULL %ss.  Indeed, this causes a #GP(0).  Despite the fact we can
         * `lcall/lret` ourselves there, and everything works fine.
         *
         * /sigh x86...
         *
         * Load a flat non-NULL %ss so test_int_handler() can sensibly return.
         */
        write_ss(GDTE_DS32_DPL0 << 3);

        /* Map gfn 0 at the very top of virtual address space. */
        nl1t[511]            = pte_from_gfn(0, PF_SYM(AD, RW, P));
        nl2t[511]            = pte_from_virt(nl1t, PF_SYM(AD, U, RW, P));
        pae_l3_identmap[511] = pte_from_virt(nl2t, PF_SYM(AD, U, RW, P));
        pae_l4_identmap[511] = pte_from_virt(pae_l3_identmap, PF_SYM(AD, U, RW, P));
        barrier();
    }
    else
    {
        unsigned long extent = (GB(4) - 1) >> PAGE_SHIFT;
        struct xen_memory_reservation mr =
        {
            .extent_start = &extent,
            .nr_extents = 1,
            .domid = DOMID_SELF,
        };

        /* Populate gfn 0xfffff (the 4GB boundary) with real RAM. */
        if ( hypercall_memory_op(XENMEM_populate_physmap, &mr) != 1 )
            panic("Failed to populate_physmap() at %#lx\n", extent);
    }

    /*
     * Copy the instruction stub over the virtual address space wrap.
     */
    uint8_t *stub = _p(~0ULL - 5);
    unsigned int res;

    extern char insn_buf_start[] asm("insn_stub_start");
    extern char insn_buf_end[] asm("insn_stub_end");
    memcpy(stub, insn_buf_start, insn_buf_end - insn_buf_start);

    /*
     * Work around suspected Broadwell TLB Erratum
     *
     * Occasionally, this test failes with:
     *
     *   --- Xen Test Framework ---
     *   Environment: HVM 64bit (Long mode 4 levels)
     *   XSA-186 PoC
     *   ******************************
     *   PANIC: Unhandled exception at 0008:fffffffffffffffa
     *   Vec 14 #PF[-I-sr-] %cr2 fffffffffffffffa
     *   ******************************
     *
     * on Broadwell hardware.  The mapping is definitely present as the
     * memcpy() has already succeeded.  Inserting an invlpg resolves the
     * issue, sugguesting that there is a race conditon between dTLB/iTLB
     * handling.
     *
     * Work around the issue for now, to avoid intermittent OSSTest failures
     * from blocking pushes of unrelated changes.
     */
    invlpg(stub);

    /*
     * Execute the stub.
     *
     * Intel CPUs are happy doing this for 32 and 64bit.  AMD CPUs are happy
     * for 64bit, but hit a segment limit violation for 32bit.  The Xen
     * emulator follows AMD behaviour for 32bit guests.
     *
     * Cover the segment limit violation with an extable handler, which
     * matches the `int $N` behaviour.  All that matters is that the
     * instructions don't get lost.
     */
    asm volatile ("call *%[ptr];"
                  _ASM_EXTABLE_HANDLER(-1, 0, ex_fault)
                  : "=a" (res)
                  : "0" (0),
                    [ptr] "r" (stub),
                    "X" (ex_fault)
                  : "memory");

    if ( res != 0xc0de )
        xtf_failure("Fail: Instruction stub wasn't executed correctly\n");

    /*
     * Execute a 16bit stub.
     *
     * Construct a 16bit unreal mode code segment with a 32bit segment limit,
     * to execute the code with.  The stub still runs with 32bit data
     * segments, which is perfectly valid.
     */
    gdt[GDTE_AVAIL0] = GDTE_SYM(0, 0xfffff, COMMON, CODE, DPL0, R);

    asm volatile ("push $%c[cs16];"
                  "push $1f;"
#ifdef __x86_64__
                  "rex64 "
#endif
                  "lret; 1:"

                  ".code16;"
                  "start_16bit:;"
                  _ASM_XEN_FEP
                  "int $" STR(X86_VEC_AVAIL) ";"
                  "ljmpl $%c[cs], $.Ldone;"
                  "end_16bit:;"
#if __x86_64__
                  ".code64;"
#else
                  ".code32;"
#endif
                  ".Ldone:"
                  : "=a" (res)
                  : "0" (0),
                    [cs] "i" (__KERN_CS),
                    [cs16] "i" (GDTE_AVAIL0 << 3)
                  : "memory");

    if ( res != 0xc0de )
        xtf_failure("Fail: 16bit code wasn't executed correctly\n");

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
