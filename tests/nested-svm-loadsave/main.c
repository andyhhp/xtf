/**
 * @file tests/nested-svm-vmloadsave/main.c
 * @ref test-nested-svm-vmloadsave
 *
 * @page test-nested-svm-vmloadsave nested-svm-vmloadsave
 *
 * Testing of AMD SVM VMLOAD and VMSAVE from an hvm64 L1 guest.
 *
 * The test exercises the reachable architectural VMLOAD and VMSAVE
 * failure cases in this harness:
 * 1. SVM disabled via EFER.SVME clear.
 * 2. Instructions executed at CPL > 0.
 * 3. Instructions executed with malformed VMCB physical addresses in RAX.
 *
 * The test passes if it receives the expected exception class for each case.
 *
 * @include tests/nested-svm-vmloadsave/index.rst
 * @see tests/nested-svm-vmloadsave/main.c
 */
#include <nested-svm/setup-l2.h>

const char test_title[] = "Nested SVM VMLOAD/VMSAVE";

enum svm_negative_mode {
    SVM_NEGATIVE_KERNEL,
    SVM_NEGATIVE_USER,
};

enum svm_negative_paddr_kind {
    SVM_NEGATIVE_PADDR_ALIGNED,
    SVM_NEGATIVE_PADDR_UNALIGNED,
    SVM_NEGATIVE_PADDR_TOO_WIDE,
};

struct svm_negative_case {
    const char *name;
    bool svme;
    enum svm_negative_mode mode;
    enum svm_negative_paddr_kind paddr_kind;
    exinfo_t expected;
};

struct svm_negative_ops {
    exinfo_t (*kernel)(uint64_t paddr);
    unsigned long (*user)(unsigned long paddr);
};

/* Return false when setup already reported a skip or failure for the case. */
typedef bool (*svm_negative_setup_fn)(const struct svm_negative_case *t,
                                      uint64_t paddr, void *data);
typedef void (*svm_negative_teardown_fn)(const struct svm_negative_case *t,
                                         uint64_t paddr, void *data);

/*
 * Execution context for a family of SVM negative cases.
 *
 * Optional setup/teardown hooks let higher-level tests prepare nested
 * state such as L2 or L3 control structures outside the fixed case matrix
 * before the helper dispatches the actual instruction.
 */
struct svm_negative_runner {
    const void *vmcb_page;
    const struct svm_negative_ops *ops;
    void *data;
    svm_negative_setup_fn setup;
    svm_negative_teardown_fn teardown;
};

/**
 * Execute VMLOAD at CPL0 with a caller-supplied VMCB physical address.
 * @param paddr Candidate VMCB physical address for RAX.
 * @return Recorded exception information, or zero on success.
 */
static exinfo_t stub_vmload(uint64_t paddr)
{
    exinfo_t fault = 0;

    asm volatile ("1: vmload %%rax; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+D" (fault)
                  : "a" (paddr), [rec] "p" (ex_record_fault_edi)
                  : "memory");

    return fault;
}

/**
 * Execute VMLOAD at CPL3 using a caller-supplied VMCB physical address.
 * @param paddr Candidate VMCB physical address for RAX.
 * @return Recorded exception information, or zero on success.
 */
static unsigned long __user_text user_vmload(unsigned long paddr)
{
    unsigned long fault = 0;

    asm volatile ("mov %[paddr], %%rax;"
                  "1: vmload %%rax; xor %%eax, %%eax; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+a" (fault)
                  : [paddr] "r" (paddr),
                    [rec] "p" (ex_record_fault_eax)
                  : "memory");

    return fault;
}

static const struct svm_negative_ops vmload_ops = {
    .kernel = stub_vmload,
    .user = user_vmload,
};

/* Aligned scratch page used to form candidate VMCB physical addresses. */
static uint8_t vmcb_page[PAGE_SIZE] __page_aligned_bss;

static const struct svm_negative_runner vmload_runner = {
    .vmcb_page = vmcb_page,
    .ops = &vmload_ops,
};

/**
 * Execute VMSAVE at CPL0 with a caller-supplied VMCB physical address.
 * @param paddr Candidate VMCB physical address for RAX.
 * @return Recorded exception information, or zero on success.
 */
static exinfo_t stub_vmsave(uint64_t paddr)
{
    exinfo_t fault = 0;

    asm volatile ("1: vmsave %%rax; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+D" (fault)
                  : "a" (paddr), [rec] "p" (ex_record_fault_edi)
                  : "memory");

    return fault;
}

/**
 * Execute VMSAVE at CPL3 using a caller-supplied VMCB physical address.
 * @param paddr Candidate VMCB physical address for RAX.
 * @return Recorded exception information, or zero on success.
 */
static unsigned long __user_text user_vmsave(unsigned long paddr)
{
    unsigned long fault = 0;

    asm volatile ("mov %[paddr], %%rax;"
                  "1: vmsave %%rax; xor %%eax, %%eax; 2:"
                  _ASM_EXTABLE_HANDLER(1b, 2b, %P[rec])
                  : "+a" (fault)
                  : [paddr] "r" (paddr),
                    [rec] "p" (ex_record_fault_eax)
                  : "memory");

    return fault;
}

static const struct svm_negative_ops vmsave_ops = {
    .kernel = stub_vmsave,
    .user = user_vmsave,
};

static const struct svm_negative_runner vmsave_runner = {
    .vmcb_page = vmcb_page,
    .ops = &vmsave_ops,
};

static uint64_t svm_negative_paddr(const void *vmcb_page,
                                   enum svm_negative_paddr_kind kind)
{
    switch ( kind )
    {
    case SVM_NEGATIVE_PADDR_ALIGNED:
        return _u(vmcb_page);

    case SVM_NEGATIVE_PADDR_UNALIGNED:
        return _u(vmcb_page) | 1ul;

    case SVM_NEGATIVE_PADDR_TOO_WIDE:
        return 1ull << maxphysaddr;
    }

    unreachable();
}

static bool svm_negative_run(const struct svm_negative_case *t,
                             const struct svm_negative_runner *runner,
                             exinfo_t *res)
{
    uint64_t paddr = svm_negative_paddr(runner->vmcb_page, t->paddr_kind);
    uint64_t orig_efer = 0;

    *res = 0;

    if ( runner->setup && !runner->setup(t, paddr, runner->data) )
        return false;

    orig_efer = update_efer_svme(t->svme);

    switch ( t->mode )
    {
    case SVM_NEGATIVE_KERNEL:
        *res = runner->ops->kernel(paddr);
        break;

    case SVM_NEGATIVE_USER:
        *res = exec_user_param(runner->ops->user, paddr);
        break;
    }

    if ( runner->teardown )
        runner->teardown(t, paddr, runner->data);

    wrmsr(MSR_EFER, orig_efer);
    return *res == t->expected;
}

bool svm_negative_check_cases(const struct svm_negative_case *cases,
                              unsigned int nr_cases,
                              const struct svm_negative_runner *runner)
{
    bool result = true;

    for ( unsigned int i = 0; i < nr_cases; ++i )
    {
        exinfo_t res;

        printk("Running test %u: %s\n", i + 1, cases[i].name);

        if ( !svm_negative_run(&cases[i], runner, &res) )
        {
            xtf_failure("Fail: %s, got %pe, expected %pe\n",
            cases[i].name, _p(res), _p(cases[i].expected));
            result = false;
        }
    }

    return result;
}

/**
 * Execute the reachable VMLOAD and VMSAVE negative-case matrices.
 *
 * The matrix covers the distinct bare-metal failure classes that can be
 * observed from an hvm64 L1 harness: EFER.SVME clear, CPL > 0, and malformed
 * VMCB physical addresses in RAX.
 */
void test_main(void)
{
    static const struct svm_negative_case vmload_cases[] = {
        {
            .name = "vmload with SVME clear at CPL0",
            .svme = false,
            .mode = SVM_NEGATIVE_KERNEL,
            .paddr_kind = SVM_NEGATIVE_PADDR_ALIGNED,
            .expected = EXINFO_SYM(UD, 0),
        },
        {
            .name = "vmload with SVME clear at CPL0 and unaligned paddr",
            .svme = false,
            .mode = SVM_NEGATIVE_KERNEL,
            .paddr_kind = SVM_NEGATIVE_PADDR_UNALIGNED,
            .expected = EXINFO_SYM(GP, 0),
        },
        {
            .name = "vmload with SVME clear at CPL0 and overly wide paddr",
            .svme = false,
            .mode = SVM_NEGATIVE_KERNEL,
            .paddr_kind = SVM_NEGATIVE_PADDR_TOO_WIDE,
            .expected = EXINFO_SYM(UD, 0),
        },
        {
            .name = "vmload with SVME clear at CPL3",
            .svme = false,
            .mode = SVM_NEGATIVE_USER,
            .paddr_kind = SVM_NEGATIVE_PADDR_ALIGNED,
            .expected = EXINFO_SYM(GP, 0),
        },
        {
            .name = "vmload with SVME clear at CPL3 and unaligned paddr",
            .svme = false,
            .mode = SVM_NEGATIVE_USER,
            .paddr_kind = SVM_NEGATIVE_PADDR_UNALIGNED,
            .expected = EXINFO_SYM(GP, 0),
        },
        {
            .name = "vmload with SVME clear at CPL3 and overly wide paddr",
            .svme = false,
            .mode = SVM_NEGATIVE_USER,
            .paddr_kind = SVM_NEGATIVE_PADDR_TOO_WIDE,
            .expected = EXINFO_SYM(GP, 0),
        },
        {
            .name = "vmload with SVME set at CPL3",
            .svme = true,
            .mode = SVM_NEGATIVE_USER,
            .paddr_kind = SVM_NEGATIVE_PADDR_ALIGNED,
            .expected = EXINFO_SYM(GP, 0),
        },
        {
            .name = "vmload with SVME set at CPL3 and unaligned paddr",
            .svme = true,
            .mode = SVM_NEGATIVE_USER,
            .paddr_kind = SVM_NEGATIVE_PADDR_UNALIGNED,
            .expected = EXINFO_SYM(GP, 0),
        },
        {
            .name = "vmload with SVME set at CPL3 and overly wide paddr",
            .svme = true,
            .mode = SVM_NEGATIVE_USER,
            .paddr_kind = SVM_NEGATIVE_PADDR_TOO_WIDE,
            .expected = EXINFO_SYM(GP, 0),
        },
        {
            .name = "vmload with SVME set at CPL0 and unaligned paddr",
            .svme = true,
            .mode = SVM_NEGATIVE_KERNEL,
            .paddr_kind = SVM_NEGATIVE_PADDR_UNALIGNED,
            .expected = EXINFO_SYM(GP, 0),
        },
        {
            .name = "vmload with SVME set at CPL0 and overly wide paddr",
            .svme = true,
            .mode = SVM_NEGATIVE_KERNEL,
            .paddr_kind = SVM_NEGATIVE_PADDR_TOO_WIDE,
            .expected = EXINFO_SYM(GP, 0),
        },
    };

    static const struct svm_negative_case vmsave_cases[] = {
        {
            .name = "vmsave with SVME clear at CPL0",
            .svme = false,
            .mode = SVM_NEGATIVE_KERNEL,
            .paddr_kind = SVM_NEGATIVE_PADDR_ALIGNED,
            .expected = EXINFO_SYM(UD, 0),
        },
        {
            .name = "vmsave with SVME clear at CPL0 and unaligned paddr",
            .svme = false,
            .mode = SVM_NEGATIVE_KERNEL,
            .paddr_kind = SVM_NEGATIVE_PADDR_UNALIGNED,
            .expected = EXINFO_SYM(GP, 0),
        },
        {
            .name = "vmsave with SVME clear at CPL0 and overly wide paddr",
            .svme = false,
            .mode = SVM_NEGATIVE_KERNEL,
            .paddr_kind = SVM_NEGATIVE_PADDR_TOO_WIDE,
            .expected = EXINFO_SYM(UD, 0),
        },
        {
            .name = "vmsave with SVME clear at CPL3",
            .svme = false,
            .mode = SVM_NEGATIVE_USER,
            .paddr_kind = SVM_NEGATIVE_PADDR_ALIGNED,
            .expected = EXINFO_SYM(GP, 0),
        },
        {
            .name = "vmsave with SVME clear at CPL3 and unaligned paddr",
            .svme = false,
            .mode = SVM_NEGATIVE_USER,
            .paddr_kind = SVM_NEGATIVE_PADDR_UNALIGNED,
            .expected = EXINFO_SYM(GP, 0),
        },
        {
            .name = "vmsave with SVME clear at CPL3 and overly wide paddr",
            .svme = false,
            .mode = SVM_NEGATIVE_USER,
            .paddr_kind = SVM_NEGATIVE_PADDR_TOO_WIDE,
            .expected = EXINFO_SYM(GP, 0),
        },
        {
            .name = "vmsave with SVME set at CPL3",
            .svme = true,
            .mode = SVM_NEGATIVE_USER,
            .paddr_kind = SVM_NEGATIVE_PADDR_ALIGNED,
            .expected = EXINFO_SYM(GP, 0),
        },
        {
            .name = "vmsave with SVME set at CPL3 and unaligned paddr",
            .svme = true,
            .mode = SVM_NEGATIVE_USER,
            .paddr_kind = SVM_NEGATIVE_PADDR_UNALIGNED,
            .expected = EXINFO_SYM(GP, 0),
        },
        {
            .name = "vmsave with SVME set at CPL3 and overly wide paddr",
            .svme = true,
            .mode = SVM_NEGATIVE_USER,
            .paddr_kind = SVM_NEGATIVE_PADDR_TOO_WIDE,
            .expected = EXINFO_SYM(GP, 0),
        },
        {
            .name = "vmsave with SVME set at CPL0 and unaligned paddr",
            .svme = true,
            .mode = SVM_NEGATIVE_KERNEL,
            .paddr_kind = SVM_NEGATIVE_PADDR_UNALIGNED,
            .expected = EXINFO_SYM(GP, 0),
        },
        {
            .name = "vmsave with SVME set at CPL0 and overly wide paddr",
            .svme = true,
            .mode = SVM_NEGATIVE_KERNEL,
            .paddr_kind = SVM_NEGATIVE_PADDR_TOO_WIDE,
            .expected = EXINFO_SYM(GP, 0),
        },
    };
    bool res1, res2;

    if ( !cpu_has_svm )
    {
        xtf_skip("Skip: SVM not available\n");
        return;
    }

    res1 = svm_negative_check_cases(vmload_cases, ARRAY_SIZE(vmload_cases),
                             &vmload_runner);
    res2 = svm_negative_check_cases(vmsave_cases, ARRAY_SIZE(vmsave_cases),
                             &vmsave_runner);
    if (res1 && res2)
        xtf_success(NULL);
    else
        xtf_failure("One or more negative cases failed\n");
}
