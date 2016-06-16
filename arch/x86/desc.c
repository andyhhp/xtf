#include <xtf/numbers.h>

#include <arch/desc.h>
#include <arch/segment.h>
#include <arch/symbolic-const.h>

user_desc gdt[NR_GDT_ENTRIES] =
{
    [GDTE_CS64_DPL0] = INIT_GDTE_SYM(0, 0xfffff, COMMON, CODE, DPL0, R, L),
    [GDTE_CS32_DPL0] = INIT_GDTE_SYM(0, 0xfffff, COMMON, CODE, DPL0, R, D),
    [GDTE_DS32_DPL0] = INIT_GDTE_SYM(0, 0xfffff, COMMON, DATA, DPL0, B, W),

    [GDTE_CS64_DPL3] = INIT_GDTE_SYM(0, 0xfffff, COMMON, CODE, DPL3, R, L),
    [GDTE_CS32_DPL3] = INIT_GDTE_SYM(0, 0xfffff, COMMON, CODE, DPL3, R, D),
    [GDTE_DS32_DPL3] = INIT_GDTE_SYM(0, 0xfffff, COMMON, DATA, DPL3, B, W),

    /* [GDTE_TSS]     */
    /* [GDTE_TSS + 1] */
};

desc_ptr gdt_ptr =
{
    .limit = sizeof(gdt) - 1,
    .base = _u(&gdt),
};

#if defined(CONFIG_HVM)

gate_desc idt[256] = { };

desc_ptr idt_ptr =
{
    .limit = sizeof(idt) - 1,
    .base = _u(&idt),
};

#endif

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
