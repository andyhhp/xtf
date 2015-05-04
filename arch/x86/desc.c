#include <arch/x86/desc.h>
#include <arch/x86/segment.h>

user_desc gdt[NR_GDT_ENTRIES] =
{
    [GDTE_CS64_DPL0] = INIT_GDTE_SYM(0, 0xfffff, COMMON, CODE, DPL0, R, L),
    [GDTE_CS32_DPL0] = INIT_GDTE_SYM(0, 0xfffff, COMMON, CODE, DPL0, R, D),
    [GDTE_DS32_DPL0] = INIT_GDTE_SYM(0, 0xfffff, COMMON, DATA, DPL0, B, W),
};

desc_ptr gdt_ptr =
{
    .limit = sizeof(gdt) - 1,
    .base = (unsigned long)&gdt,
};

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
