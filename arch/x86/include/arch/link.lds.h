/**
 * @file arch/x86/include/arch/link.lds.h
 *
 * x86 details for the linker file.
 */

#ifdef LINKER_HEADER

/* Don't clobber the ld directive */
#undef i386

#if defined(__x86_64__)

OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)

#elif defined(__i386__)

OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)

#else
# error Bad architecture to link with
#endif

ENTRY(_elf_start)

#endif /* LINKER_HEADER */

#ifdef LINKER_FOOTER

ASSERT(IS_ALIGNED(hypercall_page, PAGE_SIZE), "hypercall_page misaligned");

#ifdef CONFIG_HVM

/* Check everything lives within l1_identmap[] for user and r/o mappings. */
ASSERT(_end < (1 << (PT_ORDER + PAGE_SHIFT)), "Mappings exceed l1_identmap[]");

ASSERT(IS_ALIGNED(pae_l1_identmap, PAGE_SIZE), "pae_l1_identmap misaligned");
ASSERT(IS_ALIGNED(pae_l2_identmap, PAGE_SIZE), "pae_l2_identmap misaligned");
ASSERT(IS_ALIGNED(pae_l3_identmap, PAGE_SIZE), "pae_l3_identmap misaligned");
ASSERT(IS_ALIGNED(pae_l4_identmap, PAGE_SIZE), "pae_l4_identmap misaligned");

ASSERT(IS_ALIGNED(pae32_l3_identmap, 32), "pae32_l3_ident misaligned");

ASSERT(IS_ALIGNED(pse_l1_identmap, PAGE_SIZE), "pse_l1_identmap misaligned");
ASSERT(IS_ALIGNED(pse_l2_identmap, PAGE_SIZE), "pse_l2_identmap misaligned");

#else /* CONFIG_HVM */

ASSERT(IS_ALIGNED(gdt, PAGE_SIZE), "gdt misaligned");

#endif /* CONFIG_HVM */

#endif /* LINKER_FOOTER */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
