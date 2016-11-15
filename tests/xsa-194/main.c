/**
 * @file tests/xsa-194/main.c
 * @ref test-xsa-194
 *
 * @page test-xsa-194 XSA-194
 *
 * Advisory: [XSA-194](http://xenbits.xen.org/xsa/advisory-xsa-194.html)
 *
 * When a guest requests BSD_SYMTAB, Some versions of libelf use a packed
 * struct containing an Elf header, and three Section headers.  These headers
 * however are a union of their 32 and 64bit variants, resulting in padding
 * between the headers when building the BSD symtab for a 32bit PV guest.
 *
 * As a consequence, libelf stack gets leaked in the padding.
 *
 * Spotting leakage in the Elf header is easy.
 *
 * Spotting leakage in the Section headers are not.  All buggy versions of
 * libelf (to the time of writing) erroneously set Elf32_Ehdr.e_shentsize to
 * sizeof(Elf32_Shdr), despite actually providing sizeof(Elf64_Shdr) per
 * entry.
 *
 * We therefore cannot distinguish an older libelf which provides real 32bit
 * section headers, from a vulnerable version of libelf claiming 32bit section
 * header but actually providing 64bit headers.
 *
 * @see tests/xsa-194/main.c
 */
#include <xtf.h>

#include <xtf/elf.h>
#include <xen/elfnote.h>

const char test_title[] = "XSA-194 PoC";

ELFNOTE(Xen, XEN_ELFNOTE_BSD_SYMTAB, ".asciz \"yes\"");

int memcmpzero(const void *buf, size_t sz)
{
    const char *ptr = buf;
    size_t i;

    for ( i = 0; i < sz; ++i )
        if ( ptr[i] != 0 )
            return ptr[i];

    return 0;
}

void test_main(void)
{
    bool leak_detected = false;
    uint32_t *size = _p(ROUNDUP((unsigned long)&_end[0],
                                sizeof(unsigned long)));
    Elf32_Ehdr *ehdr = _p(size) + 4;

    if ( !(ehdr->e_ident[EI_MAG0] == ELFMAG0 &&
           ehdr->e_ident[EI_MAG1] == ELFMAG1 &&
           ehdr->e_ident[EI_MAG2] == ELFMAG2 &&
           ehdr->e_ident[EI_MAG3] == ELFMAG3) )
        return xtf_error("Error: Elf header not found\n");

    if ( ehdr->e_ident[EI_CLASS] != ELFCLASS32 )
        return xtf_error("Error: Unexpected ELF type %u\n",
                         ehdr->e_ident[EI_CLASS]);

    if ( ehdr->e_shnum != 3 )
        return xtf_error("Error: Expected 3 section headers\n");

    /*
     * libelf has some padding between an Elf32_Ehdr and the start of the
     * section header list it writes.  (Specifically, the padding until the
     * end of a Elf64_Ehdr).
     */
    if ( ehdr->e_ehsize < ehdr->e_shoff )
    {
        if ( memcmpzero(_p(ehdr) + ehdr->e_ehsize,
                        ehdr->e_shoff - ehdr->e_ehsize) )
        {
            leak_detected = true;
            xtf_failure("Fail: Data leaked after EHDR\n");
        }
    }

    if ( !leak_detected )
        xtf_success("Success: No leak detected\n");
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
