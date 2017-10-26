/**
 * @file tests/memop-seg/main.c
 * @ref test-memop-seg
 *
 * @page test-memop-seg Memory operand and segment emulation
 *
 * All memory operands operate through a segment.  Several factors control
 * which segment is actually used.
 *
 * 1. If the memory operand is encoded with a base register of @%rbp or @%rsp,
 *    the default segment is @%ss.  Otherwise, @%ds is the default.  (String
 *    instructions with two memory operands use @%es for their second
 *    operand.)

 * 2. Instructions encoded with an explicit segment override prefix will use
 *    the specified segment for their operand.  However, in 64bit mode with
 *    deprecated most of the segmentation model, the @%es, @%cs, @%ss and @%ds
 *    segment overrides are ignored and have no effect.
 *
 * This test uses the fact that @%ss can be distinguished from other segments
 * because it yields @#SS[0] on a segmentation fault, rather than @#GP[0].  We
 * construct a scenario in userspace which intends to suffer a segmentation
 * based fault.
 *
 * Tests are run for each explicit segment override prefix as well as no
 * prefix, and for each memory operand encoding of a base register, as well as
 * an absolute operand.  The resulting fault is used to check that the
 * expected segment is being used.
 *
 * @see tests/memop-seg/main.c
 */
#include <xtf.h>

#include "test.h"

const char test_title[] = "Memory operand and segment emulation tests";

static const struct test
{
    unsigned long (*fn)(unsigned long);
    const char *name;
    exinfo_t exp;
} tests[] =
{
#define GP EXINFO_SYM(GP, 0)
#define SS EXINFO_SYM(SS, 0)

#ifdef __i386__

    { stub_none_abs, "abs",    GP },
    { stub_none_eax, "(%eax)", GP },
    { stub_none_ecx, "(%ecx)", GP },
    { stub_none_edx, "(%edx)", GP },
    { stub_none_ebx, "(%ebx)", GP },
    { stub_none_esp, "(%esp)", SS },
    { stub_none_ebp, "(%ebp)", SS },
    { stub_none_esi, "(%esi)", GP },
    { stub_none_edi, "(%edi)", GP },

    { stub_es_abs, "%es:abs",    GP },
    { stub_es_eax, "%es:(%eax)", GP },
    { stub_es_ecx, "%es:(%ecx)", GP },
    { stub_es_edx, "%es:(%edx)", GP },
    { stub_es_ebx, "%es:(%ebx)", GP },
    { stub_es_esp, "%es:(%esp)", GP },
    { stub_es_ebp, "%es:(%ebp)", GP },
    { stub_es_esi, "%es:(%esi)", GP },
    { stub_es_edi, "%es:(%edi)", GP },

    { stub_cs_abs, "%cs:abs",    GP },
    { stub_cs_eax, "%cs:(%eax)", GP },
    { stub_cs_ecx, "%cs:(%ecx)", GP },
    { stub_cs_edx, "%cs:(%edx)", GP },
    { stub_cs_ebx, "%cs:(%ebx)", GP },
    { stub_cs_esp, "%cs:(%esp)", GP },
    { stub_cs_ebp, "%cs:(%ebp)", GP },
    { stub_cs_esi, "%cs:(%esi)", GP },
    { stub_cs_edi, "%cs:(%edi)", GP },

    { stub_ss_abs, "%ss:abs",    SS },
    { stub_ss_eax, "%ss:(%eax)", SS },
    { stub_ss_ecx, "%ss:(%ecx)", SS },
    { stub_ss_edx, "%ss:(%edx)", SS },
    { stub_ss_ebx, "%ss:(%ebx)", SS },
    { stub_ss_esp, "%ss:(%esp)", SS },
    { stub_ss_ebp, "%ss:(%ebp)", SS },
    { stub_ss_esi, "%ss:(%esi)", SS },
    { stub_ss_edi, "%ss:(%edi)", SS },

    { stub_ds_abs, "%ds:abs",    GP },
    { stub_ds_eax, "%ds:(%eax)", GP },
    { stub_ds_ecx, "%ds:(%ecx)", GP },
    { stub_ds_edx, "%ds:(%edx)", GP },
    { stub_ds_ebx, "%ds:(%ebx)", GP },
    { stub_ds_esp, "%ds:(%esp)", GP },
    { stub_ds_ebp, "%ds:(%ebp)", GP },
    { stub_ds_esi, "%ds:(%esi)", GP },
    { stub_ds_edi, "%ds:(%edi)", GP },

    { stub_fs_abs, "%fs:abs",    GP },
    { stub_fs_eax, "%fs:(%eax)", GP },
    { stub_fs_ecx, "%fs:(%ecx)", GP },
    { stub_fs_edx, "%fs:(%edx)", GP },
    { stub_fs_ebx, "%fs:(%ebx)", GP },
    { stub_fs_esp, "%fs:(%esp)", GP },
    { stub_fs_ebp, "%fs:(%ebp)", GP },
    { stub_fs_esi, "%fs:(%esi)", GP },
    { stub_fs_edi, "%fs:(%edi)", GP },

    { stub_gs_abs, "%gs:abs",    GP },
    { stub_gs_eax, "%gs:(%eax)", GP },
    { stub_gs_ecx, "%gs:(%ecx)", GP },
    { stub_gs_edx, "%gs:(%edx)", GP },
    { stub_gs_ebx, "%gs:(%ebx)", GP },
    { stub_gs_esp, "%gs:(%esp)", GP },
    { stub_gs_ebp, "%gs:(%ebp)", GP },
    { stub_gs_esi, "%gs:(%esi)", GP },
    { stub_gs_edi, "%gs:(%edi)", GP },

#else

    { stub_none_abs, "abs",    GP },
    { stub_none_rax, "(%rax)", GP },
    { stub_none_rcx, "(%rcx)", GP },
    { stub_none_rdx, "(%rdx)", GP },
    { stub_none_rbx, "(%rbx)", GP },
    { stub_none_rsp, "(%rsp)", SS },
    { stub_none_rbp, "(%rbp)", SS },
    { stub_none_rsi, "(%rsi)", GP },
    { stub_none_rdi, "(%rdi)", GP },
    { stub_none_r8,  "(%r8)",  GP },
    { stub_none_r9,  "(%r9)",  GP },
    { stub_none_r10, "(%r10)", GP },
    { stub_none_r11, "(%r11)", GP },
    { stub_none_r12, "(%r12)", GP },
    { stub_none_r13, "(%r13)", GP },
    { stub_none_r14, "(%r14)", GP },
    { stub_none_r15, "(%r15)", GP },

    { stub_es_abs, "%es:abs",    GP },
    { stub_es_rax, "%es:(%rax)", GP },
    { stub_es_rcx, "%es:(%rcx)", GP },
    { stub_es_rdx, "%es:(%rdx)", GP },
    { stub_es_rbx, "%es:(%rbx)", GP },
    { stub_es_rsp, "%es:(%rsp)", SS },
    { stub_es_rbp, "%es:(%rbp)", SS },
    { stub_es_rsi, "%es:(%rsi)", GP },
    { stub_es_rdi, "%es:(%rdi)", GP },
    { stub_es_r8,  "%es:(%r8)",  GP },
    { stub_es_r9,  "%es:(%r9)",  GP },
    { stub_es_r10, "%es:(%r10)", GP },
    { stub_es_r11, "%es:(%r11)", GP },
    { stub_es_r12, "%es:(%r12)", GP },
    { stub_es_r13, "%es:(%r13)", GP },
    { stub_es_r14, "%es:(%r14)", GP },
    { stub_es_r15, "%es:(%r15)", GP },

    { stub_cs_abs, "%cs:abs",    GP },
    { stub_cs_rax, "%cs:(%rax)", GP },
    { stub_cs_rcx, "%cs:(%rcx)", GP },
    { stub_cs_rdx, "%cs:(%rdx)", GP },
    { stub_cs_rbx, "%cs:(%rbx)", GP },
    { stub_cs_rsp, "%cs:(%rsp)", SS },
    { stub_cs_rbp, "%cs:(%rbp)", SS },
    { stub_cs_rsi, "%cs:(%rsi)", GP },
    { stub_cs_rdi, "%cs:(%rdi)", GP },
    { stub_cs_r8,  "%cs:(%r8)",  GP },
    { stub_cs_r9,  "%cs:(%r9)",  GP },
    { stub_cs_r10, "%cs:(%r10)", GP },
    { stub_cs_r11, "%cs:(%r11)", GP },
    { stub_cs_r12, "%cs:(%r12)", GP },
    { stub_cs_r13, "%cs:(%r13)", GP },
    { stub_cs_r14, "%cs:(%r14)", GP },
    { stub_cs_r15, "%cs:(%r15)", GP },

    { stub_ss_abs, "%ss:abs",    GP },
    { stub_ss_rax, "%ss:(%rax)", GP },
    { stub_ss_rcx, "%ss:(%rcx)", GP },
    { stub_ss_rdx, "%ss:(%rdx)", GP },
    { stub_ss_rbx, "%ss:(%rbx)", GP },
    { stub_ss_rsp, "%ss:(%rsp)", SS },
    { stub_ss_rbp, "%ss:(%rbp)", SS },
    { stub_ss_rsi, "%ss:(%rsi)", GP },
    { stub_ss_rdi, "%ss:(%rdi)", GP },
    { stub_ss_r8,  "%ss:(%r8)",  GP },
    { stub_ss_r9,  "%ss:(%r9)",  GP },
    { stub_ss_r10, "%ss:(%r10)", GP },
    { stub_ss_r11, "%ss:(%r11)", GP },
    { stub_ss_r12, "%ss:(%r12)", GP },
    { stub_ss_r13, "%ss:(%r13)", GP },
    { stub_ss_r14, "%ss:(%r14)", GP },
    { stub_ss_r15, "%ss:(%r15)", GP },

    { stub_ds_abs, "%ds:abs",    GP },
    { stub_ds_rax, "%ds:(%rax)", GP },
    { stub_ds_rcx, "%ds:(%rcx)", GP },
    { stub_ds_rdx, "%ds:(%rdx)", GP },
    { stub_ds_rbx, "%ds:(%rbx)", GP },
    { stub_ds_rsp, "%ds:(%rsp)", SS },
    { stub_ds_rbp, "%ds:(%rbp)", SS },
    { stub_ds_rsi, "%ds:(%rsi)", GP },
    { stub_ds_rdi, "%ds:(%rdi)", GP },
    { stub_ds_r8,  "%ds:(%r8)",  GP },
    { stub_ds_r9,  "%ds:(%r9)",  GP },
    { stub_ds_r10, "%ds:(%r10)", GP },
    { stub_ds_r11, "%ds:(%r11)", GP },
    { stub_ds_r12, "%ds:(%r12)", GP },
    { stub_ds_r13, "%ds:(%r13)", GP },
    { stub_ds_r14, "%ds:(%r14)", GP },
    { stub_ds_r15, "%ds:(%r15)", GP },

    { stub_fs_abs, "%fs:abs",    GP },
    { stub_fs_rax, "%fs:(%rax)", GP },
    { stub_fs_rcx, "%fs:(%rcx)", GP },
    { stub_fs_rdx, "%fs:(%rdx)", GP },
    { stub_fs_rbx, "%fs:(%rbx)", GP },
    { stub_fs_rsp, "%fs:(%rsp)", GP },
    { stub_fs_rbp, "%fs:(%rbp)", GP },
    { stub_fs_rsi, "%fs:(%rsi)", GP },
    { stub_fs_rdi, "%fs:(%rdi)", GP },
    { stub_fs_r8,  "%fs:(%r8)",  GP },
    { stub_fs_r9,  "%fs:(%r9)",  GP },
    { stub_fs_r10, "%fs:(%r10)", GP },
    { stub_fs_r11, "%fs:(%r11)", GP },
    { stub_fs_r12, "%fs:(%r12)", GP },
    { stub_fs_r13, "%fs:(%r13)", GP },
    { stub_fs_r14, "%fs:(%r14)", GP },
    { stub_fs_r15, "%fs:(%r15)", GP },

    { stub_gs_abs, "%gs:abs",    GP },
    { stub_gs_rax, "%gs:(%rax)", GP },
    { stub_gs_rcx, "%gs:(%rcx)", GP },
    { stub_gs_rdx, "%gs:(%rdx)", GP },
    { stub_gs_rbx, "%gs:(%rbx)", GP },
    { stub_gs_rsp, "%gs:(%rsp)", GP },
    { stub_gs_rbp, "%gs:(%rbp)", GP },
    { stub_gs_rsi, "%gs:(%rsi)", GP },
    { stub_gs_rdi, "%gs:(%rdi)", GP },
    { stub_gs_r8,  "%gs:(%r8)",  GP },
    { stub_gs_r9,  "%gs:(%r9)",  GP },
    { stub_gs_r10, "%gs:(%r10)", GP },
    { stub_gs_r11, "%gs:(%r11)", GP },
    { stub_gs_r12, "%gs:(%r12)", GP },
    { stub_gs_r13, "%gs:(%r13)", GP },
    { stub_gs_r14, "%gs:(%r14)", GP },
    { stub_gs_r15, "%gs:(%r15)", GP },

#endif

#undef SS
#undef GP
};

void test_main(void)
{
    /* Top bit set + 1GB.  Yields a non canonical address in 64bit. */
    unsigned long addr = (~0ul & ~(~0ul >> 1)) + GB(1);
    unsigned int i;

    /* For 32bit, use segments with a limit of 2GB. */
    if ( IS_DEFINED(CONFIG_32BIT) )
    {
        user_desc code = INIT_GDTE_SYM(0, 0x7ffff, COMMON, CODE, DPL3, R, D);
        user_desc data = INIT_GDTE_SYM(0, 0x7ffff, COMMON, DATA, DPL3, B, W);

        if ( IS_DEFINED(CONFIG_HVM) )
        {
            gdt[GDTE_AVAIL0] = code;
            gdt[GDTE_AVAIL1] = data;
        }
        else
        {
            int rc = hypercall_update_descriptor(virt_to_maddr(
                                                     &gdt[GDTE_AVAIL0]), code);

            if ( !rc )
                rc = hypercall_update_descriptor(virt_to_maddr(
                                                     &gdt[GDTE_AVAIL1]), data);

            if ( rc )
                return xtf_error("Error: Update descriptor failed: %d\n", rc);
        }

        exec_user_cs = GDTE_AVAIL0 << 3 | 3;
        exec_user_ss = GDTE_AVAIL1 << 3 | 3;
    }

    for ( i = 0; i < ARRAY_SIZE(tests); ++i )
    {
        const struct test *t = &tests[i];
        exinfo_t res;

        res = exec_user_param(t->fn, addr);
        if ( res != t->exp )
            xtf_failure("Fail: Testing '%s'\n"
                        "  expected %pe, got %pe\n",
                        t->name, _p(t->exp), _p(res));

        if ( !xtf_has_fep )
            continue;

        res = exec_user_param(t->fn, addr | 1);
        if ( res != t->exp )
            xtf_failure("Fail: Testing emulated '%s'\n"
                        "  expected %pe, got %pe\n",
                        t->name, _p(t->exp), _p(res));
    }

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
