/**
 * @file tests/pagetable-emulation/main.c
 * @ref test-pagetable-emulation
 *
 * @page test-pagetable-emulation Pagetable and pagewalk emulation
 *
 * This test aims to be a comprehensive test of all interesting bits in
 * pagetable entries, in each paging mode.
 *
 * The purpose of this test is to compare the behaviour of real hardware
 * against Xen's emulated pagewalk.
 *
 * There are several types of memory access in the x86 ISA
 *
 * 1. Data read and write.
 *
 *    Data accesses are tested with a single byte read or write (of the value
 *    0xc3) of the linear address under test, with an optional force emulate
 *    prefix before the mov instruction.
 *
 * 2. Instruction fetch.
 *
 *    Fetches acts similarly to a data reads, but have different control
 *    settings which affect whether an access succeeds or fails.  We `call`
 *    the page boundary under test, with the expectation of hitting the `ret`
 *    instruction placed there.
 *
 *    To correctly test an emulated fetch at the boundary, the force emulate
 *    prefix needs to execute successfully, and cause the resulting `%%rip`
 *    update to cross the page boundary.  To allow the FEP to execute
 *    successfully, it may not share a pagetable entry in common with the
 *    linear address under test, meaning that the page boundary under test
 *    must be between two entries in the top level pagetable.
 *
 * 3. Implicit data access.
 *
 *    An implicit access occurs as a side effect of an instruction which
 *    references a descriptor table.  This test uses `mov sel, %%fs` to
 *    trigger an implicit LDT access, with an optional force emulation prefix
 *    before the mov instruction.  @todo Figure out how to sensibly test
 *    implicit writes which need to set the descriptor Accessed bit.  This
 *    ends up being two separate memory accesses with different
 *    writeabilities, which will hit the same linear address under test.
 *
 * Overall, the following in-memory gadget is used:
 *
 *          _XEN_ASM_FEP     LDT[0] with `ret` at the start
 *     +-------------------+----------------------------+
 *     | -- 0f 0b 78 65 6e | c3 ff 00 00 00 f3 cf 00 -- |
 *     +-------------------+----------------------------+
 *                         ^ page boundary under test
 *
 * which allows us to construct all the memory accesses we would like to test.
 *
 * The primary paging mode comes from the test environment, which generates
 * some conditional behaviour based on `CONFIG_PAGING_LEVELS`.  The test
 * includes unpaged mode, because while the guest logically has `%%cr0.pg`
 * clear, the processor actually has paging enabled and Xen has some juggling
 * to do behind the scenes to cause correct behaviour to occur.
 *
 * Various observations about pagewalks:
 *
 * 1. Intel and AMD differ on the behaviour of `L4e.GLOBAL`.  Intel ignore the
 *    bit, while AMD treat it as strictly reserved.
 *
 * 2. Hardware does distinguish between instruction and data accesses, even if
 *    `EFER.NXE` or `CR4.SMEP` are clear, and the I/d flag isn't reported in
 *    the @#PF error code.
 *
 * Outstanding issues:
 *
 * 1. AMD EPYC erratum #1053
 *
 *    Intel and AMD's implementation of SMAP differes, with AMD's behaviour
 *    being better from a hypervisors point of view.  AMD have issued an
 *    erratum, but the model in this test doesn't adequately cope yet.
 *
 * @see tests/pagetable-emulation/main.c
 */
#include <xtf.h>

#include "stubs.h"

const char test_title[] = "Test pagetable-emulation";

#define LDT_SEL 0x0007 /**< Entry 0, LDT, RPL3 */

#define X86_PFEC_IMP (1u << 16)

#if CONFIG_PAGING_LEVELS == 0
# define PKRU_WD_VAL 0xaaaaaaaa
# define PKRU_AD_VAL 0x55555555
#else
# define PKRU_WD_VAL 0x80000000
# define PKRU_AD_VAL 0x40000000
#endif

typedef union
{
    unsigned int raw;
    struct {
        bool wp:1;
        bool ac:1;
        bool nx:1;
        bool smep:1;
        bool smap:1;
        bool pke:1;
        bool pkru_wd:1;
        bool pkru_ad:1;

        bool max:1; /* Must be last; upper boundary. */
    };
} __transparent pg_ctrl_t;


static bool shadow_paging;
static uint64_t pa_rsvd; /**< Bits which are reserved due to maxphysaddr. */
static paddr_t paddrs[2]; /**< Physical addresses to test. */

/** Number of individual tests performed.  Purely informative. */
static unsigned long nr_tests;

static struct test_printinfo {
    unsigned long va;
    bool active;
    bool user;
    bool force;
    const char *desc;
    pg_ctrl_t ctrl;

    intpte_t pteval;
    bool pte_printed;
} under_test;

static const struct stubs
{
    unsigned long (*read)          (unsigned long va);
    unsigned long (*implicit)      (unsigned long sel);
    unsigned long (*write)         (unsigned long va);
    unsigned long (*exec)          (unsigned long va);
    unsigned long (*read_user)     (unsigned long va);
    unsigned long (*implicit_user) (unsigned long sel);
    unsigned long (*write_user)    (unsigned long va);
    unsigned long (*exec_user)     (unsigned long va);
} native_stubs = {
    .read          = stub_read,
    .implicit      = stub_implicit,
    .write         = stub_write,
    .exec          = stub_exec,
    .read_user     = stub_read_user,
    .implicit_user = stub_implicit_user,
    .write_user    = stub_write_user,
    .exec_user     = stub_exec_user,
}, emul_stubs = {
    .read          = stub_force_read,
    .implicit      = stub_force_implicit,
    .write         = stub_force_write,
    .exec          = stub_force_exec,
    .read_user     = stub_force_read_user,
    .implicit_user = stub_force_implicit_user,
    .write_user    = stub_force_write_user,
    .exec_user     = stub_force_exec_user,
};

struct mapping_info
{
    unsigned int level, order;
    void *va;
    intpte_t *pte, *fe_pte;
    uint64_t paddr;
    bool leaf;

    union
    {
        intpte_t *ptes[4];
        struct
        {
            intpte_t *l1e, *l2e, *l3e, *l4e;
        };
    };
};

/**
 * Clear all Access and Dirty bits on the PTEs making up the linear address
 * under test, and shoot the mapping down.
 */
static void clear_ad(struct mapping_info *m)
{
    unsigned int i;

    for ( i = 0; i < ARRAY_SIZE(m->ptes); ++i )
        if ( m->ptes[i] )
            *m->ptes[i] &= ~_PAGE_AD;

    invlpg(m->va);
}

bool ex_check_pf(struct cpu_regs *regs,
                 const struct extable_entry *ex)
{
    if ( regs->entry_vector == X86_EXC_PF )
    {
        unsigned long cr2 = read_cr2();

        if ( cr2 != under_test.va )
            xtf_failure("Bad %%cr2: expected %p, got %p\n",
                        _p(under_test.va), _p(cr2));

        regs->ax = EXINFO_SYM(PF, regs->error_code);

        if ( ex->fixup )
            regs->ip = ex->fixup;
        else
        {
            if ( cpu_has_smap )
                asm volatile ("stac");

            regs->ip = *(unsigned long *)cpu_regs_sp(regs);
        }

        return true;
    }

    return false;
}

static pg_ctrl_t get_current_pg_ctrl(void)
{
    pg_ctrl_t ctrl = {};
    unsigned long cr4 = read_cr4();

    ctrl.wp   = read_cr0() & X86_CR0_WP;
    ctrl.ac   = read_flags() & X86_EFLAGS_AC;
    ctrl.nx   = rdmsr(MSR_EFER) & EFER_NXE;
    ctrl.smep = cr4 & X86_CR4_SMEP;
    ctrl.smap = cr4 & X86_CR4_SMAP;
    ctrl.pke  = cr4 & X86_CR4_PKE;

    if ( cpu_has_pku )
    {
        if ( !ctrl.pke )
            write_cr4(cr4 | X86_CR4_PKE);

        unsigned int pkru = rdpkru();

        ctrl.pkru_wd = pkru & PKRU_WD_VAL;
        ctrl.pkru_ad = pkru & PKRU_AD_VAL;

        if ( !ctrl.pke )
            write_cr4(cr4);
    }

    return ctrl;
}

static void __printf(1, 2) fail(const char *fmt, ...)
{
    va_list args;

    if ( !under_test.active )
        return;

    if ( !under_test.pte_printed )
    {
        intpte_t pte = under_test.pteval;
        printk("  PTE:  %"PRIpte" =>%s%s%s%s%s%s%s%s%s\n", pte,
               pte & _PAGE_NX              ? " Nx" : "",
               pte & 0x7800000000000000ull ? " Pk" : "",
               pte & 0x07f0000000000000ull ? " Av" : "",
               pte & pa_rsvd               ? " Rs" : "",
               pte & _PAGE_GLOBAL          ? " G"  : "",
               pte & _PAGE_PSE             ? " +"  : "",
               pte & _PAGE_USER            ? " U"  : "",
               pte & _PAGE_RW              ? " W"  : "",
               pte & _PAGE_PRESENT         ? " P"  : "");

        pg_ctrl_t ctrl = under_test.ctrl;
        printk("  CTRL:%s%s%s%s%s%s%s%s\n",
               ctrl.pkru_ad ? " PKAD" : "",
               ctrl.pkru_wd ? " PKWD" : "",
               ctrl.pke     ? " PKE"  : "",
               ctrl.smap    ? " SMAP" : "",
               ctrl.smep    ? " SMEP" : "",
               ctrl.nx      ? " NX"   : "",
               ctrl.ac      ? " AC"   : "",
               ctrl.wp      ? " WP"   : "");

        pg_ctrl_t curr = get_current_pg_ctrl();
        if ( ctrl.raw != curr.raw )
            panic("Bad paging ctrl: Expected %#x, got %#x\n",
                  ctrl.raw, curr.raw);

        under_test.pte_printed = true;
    }

    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);
    xtf_failure(NULL);

    static unsigned nr_failures;
    if ( nr_failures >= 16 )
    {
        printk("Excessive failures - giving up...\n");
        xtf_exit();
    }
    nr_failures++;
}

bool do_unhandled_exception(struct cpu_regs *regs)
{
    fail("ERROR: Unhandled exception during %s %s %s\n",
         under_test.force ? "Emulated" : "Native",
         under_test.user  ? "User"     : "Supervisor",
         under_test.desc);
    return false;
}

static void prepare_virtual_mappings(
    struct mapping_info *m, unsigned int level, bool super, paddr_t paddr)
{
    static intpte_t l1t[L1_PT_ENTRIES] __aligned(PAGE_SIZE);
    static intpte_t l2t[L2_PT_ENTRIES] __aligned(PAGE_SIZE);

#if CONFIG_PAGING_LEVELS > 3
    static intpte_t l3t[L3_PT_ENTRIES] __aligned(PAGE_SIZE);
#else
    extern intpte_t l3t[];
#endif

    bool pse36 = CONFIG_PAGING_LEVELS == 2 && paddr != (uint32_t)paddr;

    memset(m, 0, sizeof(*m));

#define PF_PGTBL PF_SYM(AD, U, RW, P)
    if ( CONFIG_PAGING_LEVELS == 4 )
    {

        pae_l4_identmap[1] = pte_from_virt(l3t, PF_PGTBL);
        pae_l4_identmap[2] = pte_from_virt(l3t, PF_PGTBL);

        l3t[0]   = pte_from_virt(l2t, PF_PGTBL);
        l3t[511] = pte_from_virt(l2t, PF_PGTBL);

        l2t[0]   = pte_from_virt(l1t, PF_PGTBL);
        l2t[511] = pte_from_virt(l1t, PF_PGTBL);

        l1t[0]   = pte_from_paddr(paddr,     PF_PGTBL);
        l1t[511] = pte_from_paddr(paddr - 1, PF_PGTBL);

        m->va     = _p(2ull << PAE_L4_PT_SHIFT);
        m->l1e    = &l1t[0];
        m->l2e    = &l2t[0];
        m->l3e    = &l3t[0];
        m->l4e    = _p(&pae_l4_identmap[2]);
        m->fe_pte = &l1t[511];

        asm(_ASM_EXTABLE_HANDLER(2 << PAE_L4_PT_SHIFT, 0, ex_check_pf));
    }
    else if ( CONFIG_PAGING_LEVELS == 3 )
    {
        pae32_l3_identmap[1] = pte_from_virt(l2t, _PAGE_PRESENT);
        pae32_l3_identmap[2] = pte_from_virt(l2t, _PAGE_PRESENT);

        l2t[0]   = pte_from_virt(l1t, PF_PGTBL);
        l2t[511] = pte_from_virt(l1t, PF_PGTBL);

        l1t[0]   = pte_from_paddr(paddr,     PF_PGTBL);
        l1t[511] = pte_from_paddr(paddr - 1, PF_PGTBL);

        m->va     = _p(2ull << PAE_L3_PT_SHIFT);
        m->l1e    = &l1t[0];
        m->l2e    = &l2t[0];
        m->l3e    = _p(&pae32_l3_identmap[2]);
        m->l4e    = NULL;
        m->fe_pte = &l1t[511];

        asm(_ASM_EXTABLE_HANDLER(2 << PAE_L3_PT_SHIFT, 0, ex_check_pf));
    }
    else if ( CONFIG_PAGING_LEVELS == 2 )
    {
        if ( pse36 )
        {
            ASSERT(super);
            ASSERT(IS_ALIGNED(paddr, MB(4)));

            pse_l2_identmap[511] =
                fold_pse36((paddr - MB(4)) | PF_PGTBL | _PAGE_PSE);
            pse_l2_identmap[512] =
                fold_pse36(paddr | PF_PGTBL | _PAGE_PSE);
        }
        else
        {
            pse_l2_identmap[511] = pte_from_virt(l1t, PF_PGTBL);
            pse_l2_identmap[512] = pte_from_virt(l1t, PF_PGTBL);

            l1t[0]    = pte_from_paddr(paddr,     PF_PGTBL);
            l1t[1023] = pte_from_paddr(paddr - 1, PF_PGTBL);
        }

        m->va     = _p(2ull << PAE_L3_PT_SHIFT);
        m->l1e    = pse36 ? NULL : &l1t[0];
        m->l2e    = _p(&pse_l2_identmap[512]);
        m->l3e    = NULL;
        m->l4e    = NULL;
        m->fe_pte = pse36 ? _p(&pse_l2_identmap[511]) : &l1t[1023];

        asm(_ASM_EXTABLE_HANDLER(2 << PAE_L3_PT_SHIFT, 0, ex_check_pf));
    }
    else
        panic("%s() PAGING_LEVELS %u not implemented yet\n",
              __func__, CONFIG_PAGING_LEVELS);

#undef PF_PGTBL

    /* Flush the TLB before trying to use the new mappings. */
    flush_tlb(false);

    m->level = level;
    m->pte = m->ptes[level - 1];

    if ( pse36 )
    {
        /* No l1e at all. */
        m->order = PT_ORDER + PAGE_SHIFT;
        m->paddr = *m->pte & ~0xfff;
        m->leaf  = true;
    }
    else if ( super && (read_cr4() & (X86_CR4_PAE|X86_CR4_PSE)) )
    {
        /* Superpage in effect. */
        m->order = ((level - 1) * PT_ORDER) + PAGE_SHIFT;
        m->paddr =  *m->l1e & ~0xfff;
        m->leaf  = true;
    }
    else
    {
        /* Small page, or superpage not in effect. */
        m->order = 0;
        m->paddr = *m->pte & ~0xfff;
        m->leaf  = (level == 1);
    }
}

static void check(struct mapping_info *m, exinfo_t actual, exinfo_t expected,
                  unsigned int walk)
{
    /* Check that the actual pagefault matched our expectation. */
    if ( actual != expected )
        fail("    Fail: expected %pe, got %pe for %s %s %s\n",
             _p(expected), _p(actual),
             under_test.force ? "Emulated" : "Native",
             under_test.user  ? "User"     : "Supervisor",
             under_test.desc);

    /* Check that A/D bits got updated as expected. */
    unsigned int leaf_level =
        m->order ? ((m->order - PAGE_SHIFT) / PT_ORDER) : 0;
    unsigned int i; /* NB - Levels are 0-indexed. */

    if ( vendor_is_amd )
    {
        /*
         * On AMD hardware, updates to A/D bits are not ordered with respect
         * to ordinary memory accesses.  They are ordered by atomic
         * operations, serialising events, and mfence.  mfence is by far the
         * lowsest overhead option when running on shadow pagetables, due to
         * not causing a vmexit.
         */
        mb();
    }

    for ( i = 0; i < ARRAY_SIZE(m->ptes); ++i )
    {
        int exp_a, exp_d;

        if ( !m->ptes[i] )
            continue;

        if ( CONFIG_PAGING_LEVELS == 3 && i == 2 )
        {
            /*
             * 32bit PAE paging is special.  The 4 PDPTE's are read from
             * memory, cached in the processor and don't strictly count as
             * pagetables. The A/D bits are not updated.
             */
            exp_a = 0;
            exp_d = 0;
        }
        else if ( leaf_level > i )
        {
            /*
             * Logically below a superpage.  Nothing should consider this part
             * of the pagetable structure, and neither A or D should be set.
             */
            exp_a = 0;
            exp_d = 0;
        }
        else if ( leaf_level == i )
        {
            /*
             * At a leaf page.  If there was no fault, we expect A to be set,
             * optionally D if a write occurred.
             */
            exp_a = (actual == 0);
            exp_d = exp_a && (walk & PFEC_SYM(W));
        }
        else
        {
            /*
             * Higher level translation structure.  A processor is free to
             * cache the partial translation or not, at its discretion, but
             * they will never be dirty.
             */
            exp_a = -1;
            exp_d = 0;
        }

        bool act_a = *m->ptes[i] & _PAGE_ACCESSED;
        bool act_d = *m->ptes[i] & _PAGE_DIRTY;

        if ( (exp_a >= 0 && exp_a != act_a) || (exp_d != act_d) )
            fail("    Fail: expected L%u AD = %c%u, got %u%u for %s %s\n",
                 i + 1, exp_a == 1 ? '1' : exp_a == 0 ? '0' : 'x', exp_d,
                 act_a, act_d,
                 under_test.user ? "User" : "Supervisor", under_test.desc);
    }

    clear_ad(m);
    write_fs(0);
}

static exinfo_t calc_fault(struct mapping_info *m, uint64_t pte,
                           unsigned int walk, pg_ctrl_t ctrl)

{
    bool nx_valid = CONFIG_PAGING_LEVELS >= 3 && ctrl.nx;
    uint64_t rsvd = pa_rsvd;

    /* Accumulate additional bits which are reserved. */
    if ( !nx_valid )
        rsvd |= _PAGE_NX;

    if ( m->level == 4 )
        rsvd |= _PAGE_PSE | (vendor_is_amd ? _PAGE_GLOBAL : 0);
    else if ( m->level == 3 && !cpu_has_page1gb )
        rsvd |= _PAGE_PSE;

    if ( m->order )
    {
        if ( CONFIG_PAGING_LEVELS > 2 || !cpu_has_pse36 )
            rsvd |= ((1ull << m->order) - 1) & ~(_PAGE_PSE_PAT | (_PAGE_PSE_PAT - 1));
        else
            rsvd |= (1ull << 21) | fold_pse36(rsvd);
    }

    if ( CONFIG_PAGING_LEVELS == 3 )
        rsvd |= 0x7ff0000000000000ull;

    /*
     * A pagefault always reports the User and Writeable input from the
     * pagewalk.
     */
    exinfo_t fault = EXINFO_SYM(PF, walk & PFEC_SYM(U, W));

    /*
     * Instruction fetch (vs data access) is only reported when NX or SMEP are
     * active.
     */
    if ( nx_valid || ctrl.smep )
        fault |= (walk & PFEC_SYM(I));

    /* Check whether a translation exists. */
    if ( !(pte & _PAGE_PRESENT) )
        return fault;

    /* When a translation exists, Present is always reported. */
    fault |= PFEC_SYM(P);

    /* Check whether any reserved bits are set. */
    if ( pte & rsvd )
        return fault | PFEC_SYM(R);

    /* Check access rights. */

    if ( walk & PFEC_SYM(I) ) /* Instruction fetch. */
    {
        if ( pte & _PAGE_NX )
            return fault; /* Fetch from NX page. */

        if ( walk & PFEC_SYM(U) ) /* User fetch. */
        {
            if ( !(pte & _PAGE_USER) )
                return fault; /* User fetch from supervisor page. */
        }
        else /* Supervisor fetch. */
        {
            if ( (pte & _PAGE_USER) && ctrl.smep )
                return fault; /* Supervisor fetch from user page with SMEP. */
        }
    }
    else /* Data access */
    {
        if ( walk & PFEC_SYM(U) ) /* User access. */
        {
            if ( CONFIG_PAGING_LEVELS == 4 && ctrl.pke &&
                 m->leaf && (((pte >> 59) & 0xf) == 0xf) )
            {
                if ( ctrl.pkru_ad ||
                     ((walk & PFEC_SYM(W)) && ctrl.pkru_wd &&
                      ((walk & PFEC_SYM(U)) || ctrl.wp)) )
                    return fault | PFEC_SYM(K); /* Protection key. */
            }

            if ( !(pte & _PAGE_USER) )
                return fault; /* User access from supervisor page. */

            if ( (walk & PFEC_SYM(W)) && !(pte & _PAGE_RW) )
                return fault; /* Write to a read-only page. */
        }
        else /* Supervisor access. */
        {
            if ( (walk & PFEC_SYM(W)) && !(pte & _PAGE_RW) && ctrl.wp )
                return fault; /* Write to a read-only page with WP. */

            if ( (pte & _PAGE_USER) && ctrl.smap &&
                 ((walk & PFEC_SYM(IMP)) || !ctrl.ac) )
                return fault; /* Supervisor access from user page with SMAP and
                               * (Implicit or !AC)? */
        }
    }

    /* Should succeed. */
    return 0;
}

static void test_pte(struct mapping_info *m, uint64_t overlay, pg_ctrl_t ctrl)
{
    uint64_t new = m->paddr | overlay;
    bool user;
    const struct stubs *stubs;

    under_test = (struct test_printinfo){
        .pteval = new,
        .active = true,
        .va = _u(m->va),
        .ctrl = ctrl,
    };

    *m->pte = new;
    clear_ad(m);

    for ( user = false; ; user = true )
    {
        unsigned int walk, base = user ? PFEC_SYM(U) : 0;

        under_test.user = user;

#define CALL(fn, va)                                                \
        (user ? exec_user_param(fn ## _user, _u(va)) : fn(_u(va)))

        for ( stubs = &native_stubs; ; stubs = &emul_stubs )
        {
            under_test.force = stubs == &emul_stubs;

            /* Map the exec FEP stub with suitable permissions. */
            if ( under_test.force )
            {
                if ( user )
                    *m->fe_pte |= _PAGE_USER;
                else
                    *m->fe_pte &= ~_PAGE_USER;
                invlpg(m->va - 5);
            }

            /* Basic read. */
            under_test.desc = "Read";
            walk = base | PFEC_SYM();
            check(m, CALL(stubs->read, m->va),
                  calc_fault(m, new, walk, ctrl), walk);

            /* Implicit read (always supervisor).  `mov $LDT_SEL, %fs`. */
            if ( user || !shadow_paging || !(ctrl.smap && ctrl.ac) )
            {
                under_test.desc = "Implicit Read";
                walk = PFEC_SYM(IMP);
                check(m, CALL(stubs->implicit, LDT_SEL),
                      calc_fault(m, new, walk, ctrl), walk);
            }
            else
                nr_tests--; /* Adjust later += 4 for skipping this test. */

            /* Basic write. */
            under_test.desc = "Write";
            walk = base | PFEC_SYM(W);
            check(m, CALL(stubs->write, m->va),
                  calc_fault(m, new, walk, ctrl), walk);

            /* Basic exec. */
            under_test.desc = "Exec";
            walk = base | PFEC_SYM(I);
            check(m, CALL(stubs->exec, m->va),
                  calc_fault(m, new, walk, ctrl), walk);

            nr_tests += 4;

            if ( !xtf_has_fep || stubs == &emul_stubs )
                break;
        }

        if ( user )
            break;
    }

#undef CALL

    under_test.active = false;
}

static void shatter_console_superpage(void)
{
    /*
     * Shatter the superpage mapping the PV console. We want to test with
     * CR4.PSE disabled, at which point superpages stop working.
     */
    uint64_t raw_pfn;

    if ( hvm_get_param(HVM_PARAM_CONSOLE_PFN, &raw_pfn) == 0 )
    {
        unsigned int l2o = l2_table_offset(raw_pfn << PAGE_SHIFT);

        if ( (l2_identmap[l2o] & PF_SYM(PSE, P)) == PF_SYM(PSE, P) )
        {
            static intpte_t conl1t[L1_PT_ENTRIES] __aligned(PAGE_SIZE);
            paddr_t base_gfn = l2_identmap[l2o] >> PAGE_SHIFT;
            unsigned int i;

            for ( i = 0; i < ARRAY_SIZE(conl1t); ++i )
                conl1t[i] = pte_from_gfn(base_gfn + i, PF_SYM(AD, RW, P));

            l2_identmap[l2o] = pte_from_virt(conl1t, PF_SYM(AD, U, RW, P));
        }
    }

    flush_tlb(true);
}

static void foreach_pte_bit_combination(unsigned int level, bool super,
                                        const pg_ctrl_t ctrl)
{
    unsigned long cr4 = read_cr4();
    const uint64_t base = super ? _PAGE_PSE : 0;
    struct mapping_info m;
    unsigned int i;

    for ( i = 0; i < ARRAY_SIZE(paddrs); ++i )
    {
        paddr_t paddr = paddrs[i];

        /* Skip unencodeable combinations. */
        if ( CONFIG_PAGING_LEVELS == 2 && paddr != (uint32_t)paddr &&
             (level == 1 || !cpu_has_pse36 || !(cr4 & X86_CR4_PSE) || !super) )
            continue;

        struct
        {
            bool cond;
            uint64_t bit;
        } trans_bits[] = {
            { true, 0 },
            { true, _PAGE_GLOBAL },

#if CONFIG_PAGING_LEVELS == 2

            { super && (cr4 & X86_CR4_PSE) && !cpu_has_pse36, 1ull << 13 },
            { super && (cr4 & X86_CR4_PSE) && !cpu_has_pse36, 1ull << 21 },

            { super && paddr != (uint32_t)paddr, 1ull << 21 },

            { super && paddr != (uint32_t)paddr && maxphysaddr < 39,
              fold_pse36(1ull << 39) },
            { super && paddr != (uint32_t)paddr && maxphysaddr < 38,
              fold_pse36(1ull << maxphysaddr) },

#else

            { super, 1ull << 13 },
            { super, PAGE_SIZE << (((level - 1) * PT_ORDER) - 1) },

            { maxphysaddr < 50, 1ull << maxphysaddr },
            { maxphysaddr < 51, 1ull << 51 },
            { true, 1ull << 52 },
            { true, 0xfull << 59 }, /* PKEY 15 */
            { true, _PAGE_NX },

#endif
        };

        prepare_virtual_mappings(&m, level, super, paddr);

        /* Basic access rights are the bottom 3 bits in any pagetable entry. */
        for ( unsigned int ar = 0; ar <= PF_SYM(U, RW, P); ++ar )
        {
            for ( unsigned idx = 0; idx < ARRAY_SIZE(trans_bits); ++idx )
            {
                if ( !trans_bits[idx].cond )
                    continue;

                test_pte(&m, base | trans_bits[idx].bit | ar, ctrl);
            }
        }
    }
}

static void foreach_pg_ctrl_setting(unsigned int level, bool super)
{
    unsigned long ctrl, max = (pg_ctrl_t){ .max = true, }.raw;
    pg_ctrl_t missing = {};

    missing.nx =   !cpu_has_nx;
    missing.smep = !cpu_has_smep;
    missing.smap = !cpu_has_smap;
    missing.pke =  missing.pkru_wd = missing.pkru_ad = !cpu_has_pku;

    uint64_t efer     = rdmsr(MSR_EFER);
    unsigned long cr0 = read_cr0();
    unsigned long cr4 = read_cr4();

    for ( ctrl = 0; ctrl < max; ++ctrl )
    {
        pg_ctrl_t c = { ctrl };

        if ( ctrl & missing.raw )
        {
            ctrl += ((ctrl & missing.raw) - 1);
            continue;
        }

        uint64_t nefer = (efer & ~EFER_NXE) | (c.nx ? EFER_NXE : 0);
        if ( nefer != efer )
            wrmsr(MSR_EFER, efer = nefer);

        unsigned long ncr0 = (cr0 & ~X86_CR0_WP) | (c.wp ? X86_CR0_WP : 0);
        if ( ncr0 != cr0 )
            write_cr0(cr0 = ncr0);

        unsigned long ncr4 =
            ((cr4 & ~(X86_CR4_SMEP | X86_CR4_SMAP | X86_CR4_PKE)) |
             (c.smep ? X86_CR4_SMEP : 0) |
             (c.smap ? X86_CR4_SMAP : 0) |
             ((c.pke || c.pkru_wd || c.pkru_ad) ? X86_CR4_PKE : 0));
        if ( ncr4 != cr4 )
            write_cr4(cr4 = ncr4);

        if ( cpu_has_smap )
        {
            if ( c.ac )
                asm volatile ("stac");
            else
                asm volatile ("clac");
        }
        else
            asm volatile ("pushf;"
                          "andl $~" STR(X86_EFLAGS_AC) ", (%%" _ASM_SP ");"
                          "orl  %[ac], (%%" _ASM_SP ");"
                          "popf"
                          :: [ac] "r" (c.ac ? X86_EFLAGS_AC : 0));

        if ( cr4 & X86_CR4_PKE )
        {
            wrpkru((c.pkru_wd ? PKRU_WD_VAL : 0) |
                   (c.pkru_ad ? PKRU_AD_VAL : 0));
            if ( !c.pke )
                write_cr4(cr4 = (cr4 & ~X86_CR4_PKE));
        }

        if ( CONFIG_PAGING_LEVELS == 0 )
            ;
        else
            foreach_pte_bit_combination(level, super, c);
    }

    /* Reset environment. */
    wrmsr(MSR_EFER, efer & ~EFER_NXE);
    write_cr0(cr0 & ~X86_CR0_WP);
    if ( cpu_has_pku )
    {
        if ( !(cr4 & X86_CR4_PKE) )
            write_cr4(cr4 | X86_CR4_PKE);
        wrpkru(0);
    }
    write_cr4(cr4 & ~(X86_CR4_SMEP | X86_CR4_SMAP | X86_CR4_PKE));
}

static void test_pagetable_level(unsigned int level, bool super)
{
    printk("Test L%ue%s%s\n",
           level, super                 ? " Superpage" : "",
           CONFIG_PAGING_LEVELS == 2 &&
           !(read_cr4() & X86_CR4_PSE)  ? " (No PSE)"  : "");

    foreach_pg_ctrl_setting(level, super);
}

static void foreach_pagetable_level(void)
{
    test_pagetable_level(1, false);

    if ( CONFIG_PAGING_LEVELS == 2 )
    {
        unsigned long cr4 = read_cr4();

        write_cr4(cr4 & ~X86_CR4_PSE);

        test_pagetable_level(2, false);
        test_pagetable_level(2, true);

        write_cr4(cr4 | X86_CR4_PSE);
    }

    test_pagetable_level(2, false);
    test_pagetable_level(2, true);

    if ( CONFIG_PAGING_LEVELS > 3 )
    {
        test_pagetable_level(3, false);
        test_pagetable_level(3, true);
        test_pagetable_level(4, false);
        test_pagetable_level(4, true);
    }
}

/**
 * Calculate suitable physical addresses to use, and ensure there is ram
 * behind them.  The low paddr is needs sufficient alignment to test an L3
 * superpage if necessary, while the high paddr is chosen so the maximum
 * usable physical address bit is set.
 */
static void prepare_physical_addresses(void)
{
    paddrs[0] = GB(1);
    paddrs[1] = 1ull << (min(maxphysaddr,
                             CONFIG_PAGING_LEVELS == 2
                             ? 40U
                             : (BITS_PER_LONG + PAGE_SHIFT)) - 1);

    unsigned long extents[] = {
        (paddrs[0] >> PAGE_SHIFT) - 1, paddrs[0] >> PAGE_SHIFT,
        (paddrs[1] >> PAGE_SHIFT) - 1, paddrs[1] >> PAGE_SHIFT,
    };
    struct xen_memory_reservation mr = {
        .extent_start = extents,
        .nr_extents = ARRAY_SIZE(extents),
        .domid = DOMID_SELF,
    };
    int rc = hypercall_memory_op(XENMEM_populate_physmap, &mr);

    if ( rc != ARRAY_SIZE(extents) )
        return xtf_error("Failed populate_physmap: %d\n", rc);

    unsigned int i;
    for ( i = 0; i < ARRAY_SIZE(paddrs); ++i )
    {
        struct mapping_info m;
        paddr_t paddr = paddrs[i];

        if ( CONFIG_PAGING_LEVELS == 2 && paddr != (uint32_t)paddr )
        {
            if ( cpu_has_pse36 )
                prepare_virtual_mappings(&m, 2, true, paddr);
            else
            {
                printk("Unable to prepare paddr %"PRIpaddr"\n", paddr);
                continue;
            }
        }
        else
            prepare_virtual_mappings(&m, 1, false, paddr);

        /* Put FEP immediately before va, and a ret instruction at va. */
        memcpy(m.va - 5, "\x0f\x0bxen\xc3", 6);
        barrier();

        /* Read them back, to confirm that RAM is properly in place. */
        if ( memcmp(m.va - 5, "\x0f\x0bxen\xc3", 6) )
            panic("Bad phys or virtual setup\n");

        /* Construct the LDT at va. */
        user_desc *ldt = m.va;

        /* Bottom byte of the LDT must be a `ret`, as it is also executed. */
        ldt[LDT_SEL >> 3] = GDTE_SYM(0, 0xfffc3, COMMON, DATA, DPL3, B, W);
        pack_ldt_desc(&gdt[GDTE_AVAIL0], ldt, 7);

        lldt(GDTE_AVAIL0 << 3);

        /* Check that we can successfully load the LDT selector. */
        write_fs(LDT_SEL);
    }
}

static void probe_paging_mode(void)
{
    /*
     * Shadow paging vs hap should be indistinguishable to guests.
     *
     * Shadow paging doesn't support PSE36, so this feature is nominally
     * hidden from guests.
     *
     * Luckily(?), because old versions of HyperV refuse to boot if they don't
     * see PSE36, it is purposefully leaked once PAE is enabled to keep HyperV
     * happy.
     *
     * As a result, our shadow paging heuristic is that the visibility of the
     * PSE36 feature changes as we flip in and out of PAE paging mode.
     */
    unsigned int _1d;
    unsigned long cr0 = read_cr0(), cr4 = read_cr4();

    switch ( CONFIG_PAGING_LEVELS )
    {
    case 0:
        write_cr4(cr4 | X86_CR4_PAE);
        write_cr3(_u(pae32_l3_identmap));
        write_cr0(cr0 | X86_CR0_PG);

        _1d = cpuid_edx(1);

        write_cr0(cr0);
        write_cr4(cr4);
        break;

    case 2:
        write_cr0(cr0 & ~X86_CR0_PG);

        write_cr3(_u(pae32_l3_identmap));

        write_cr4(cr4 | X86_CR4_PAE);
        write_cr0(cr0);

        _1d = cpuid_edx(1);

        write_cr0(cr0 & ~X86_CR0_PG);
        write_cr4(cr4 & ~X86_CR4_PAE);

        write_cr3(_u(cr3_target));

        write_cr0(cr0);
        break;

    case 3:
        write_cr0(cr0 & ~X86_CR0_PG);
        write_cr4(cr4 & ~X86_CR4_PAE);

        write_cr3(_u(pse_l2_identmap));

        write_cr0(cr0);

        _1d = cpuid_edx(1);

        write_cr0(cr0 & ~X86_CR0_PG);

        write_cr3(_u(cr3_target));

        write_cr4(cr4);
        write_cr0(cr0);
        break;

    case 4:
        /* Easier to write in ASM, as it involves dropping to 32bit mode. */
        _1d = stub_64bit_probe_paging_mode();
        break;
    }

    shadow_paging = cpu_has_pse36 ^ !!(_1d & cpufeat_mask(X86_FEATURE_PSE36));

    printk("  Paging mode heuristic: %s\n", shadow_paging ? "Shadow" : "Hap");
}

void test_main(void)
{
    if ( !vendor_is_intel && !vendor_is_amd )
        xtf_warning("Unknown CPU vendor.  Something might go wrong\n");

    printk("  Info: %s, Fam %u, Model %u, Stepping %u, paddr %u, vaddr %u\n"
           "  Features:%s%s%s%s%s%s%s%s%s%s%s\n",
           x86_vendor_name(x86_vendor),
           x86_family, x86_model, x86_stepping, maxphysaddr, maxvirtaddr,
           cpu_has_pse     ? " PSE"    : "",
           cpu_has_pae     ? " PAE"    : "",
           cpu_has_pge     ? " PGE"    : "",
           cpu_has_pat     ? " PAT"    : "",
           cpu_has_pse36   ? " PSE36"  : "",
           cpu_has_pcid    ? " PCID"   : "",
           cpu_has_nx      ? " NX"     : "",
           cpu_has_page1gb ? " PAGE1G" : "",
           cpu_has_smep    ? " SMEP"   : "",
           cpu_has_smap    ? " SMAP"   : "",
           cpu_has_pku     ? " PKU"    : "");

    pa_rsvd = ((1ull << 52) - 1) & ~((1ull << maxphysaddr) - 1);

    if ( !xtf_has_fep )
        xtf_skip("FEP support not detected - some tests will be skipped\n");

    probe_paging_mode();

    if ( CONFIG_PAGING_LEVELS == 0 )
    {
        printk("Test control settings in unpaged environment\n");

        foreach_pg_ctrl_setting(0, false);

        return xtf_success(NULL);
    }

    if ( CONFIG_PAGING_LEVELS == 2 )
        shatter_console_superpage();

    /* Sanitise environment. */
    wrmsr(MSR_EFER, rdmsr(MSR_EFER) & ~EFER_NXE);
    write_cr0(read_cr0() & ~X86_CR0_WP);
    write_cr4(read_cr4() & ~(X86_CR4_SMEP | X86_CR4_SMAP | X86_CR4_PKE));

    prepare_physical_addresses();

    printk("  Using physical addresses %"PRIpaddr" and %"PRIpaddr"\n",
           paddrs[0], paddrs[1]);

    foreach_pagetable_level();

    printk("Completed %lu tests\n", nr_tests);
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
