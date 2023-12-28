/**
 * @file tests/lm-ts/main.c
 * @ref test-lm-ts
 *
 * @page test-lm-ts Long Mode @#TS exception
 *
 * Task Switches are removed from the AMD64 spec, but contrary to popular
 * belief, these are not the only source of @#TS exceptions.  This is a
 * demonstration of @#TS occurring in 64bit mode (and 32bit to show that the
 * behaviour is consistent.)
 *
 * @#TS can occur because of a limit violation accessing @%tr.  Construct a
 * scenario where the TSS limit includes {e,r}sp0 (so we can handle exceptions
 * from lower privileges), but excludes {e,r}sp1 (so we can trigger @#TS).
 *
 * @see tests/lm-ts/main.c
 */
#include <xtf.h>

const char test_title[] = "Test Long Mode #TS";

#ifdef __i386__
# define COND(_32, _64) _32
#else
# define COND(_32, _64) _64
#endif

static bool check_ts(struct cpu_regs *regs, const struct extable_entry *ex)
{
    exinfo_t got = EXINFO(regs->entry_vector, regs->error_code);
    exinfo_t exp = EXINFO_SYM(TS, SEL_EC_SYM(TSS_SEL, GDT));

    if ( got == exp )
    {
        printk("  Got %pe as expected\n", _p(got));
        regs->ip = ex->fixup;
        return true;
    }

    return false;
}

static void __user_text user(void)
{
    asm volatile ("1: int $"STR(X86_VEC_AVAIL)";"
                  "2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[check])
                  :
                  : [check] "p" (check_ts)
                  : "memory");
}

void test_main(void)
{
    /*
     * Edit the TSS descriptor in place.  Reset Busy to Available, and reduce
     * limit to exclude the Ring1 stack.
     */
    gdt[GDTE_TSS].type = 0x9;
    gdt[GDTE_TSS].limit0 = offsetof(env_tss, COND(esp1, rsp1)) - 1;

    /* Reload %tr */
    ltr(TSS_SEL);

#ifdef __i386__
    tss.ss1 = __KERN_CS;
#endif

    /* Ring1 code segment. */
    gdt[GDTE_AVAIL0] = GDTE_SYM(0, 0xfffff, COMMON, DPL1, CODE, COND(D, L));

    /*
     * DPL3 RPL1 gate to allow userspace to invoke Ring 1.  It will fault as
     * %tr->{e,r}sp1 can't be accessed.
     */
    pack_intr_gate(&idt[X86_VEC_AVAIL], (GDTE_AVAIL0 * 8) | 1, 0, 3, 0);

    exec_user_void(user);

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
