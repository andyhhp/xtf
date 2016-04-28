/**
 * @file arch/x86/decode.c
 *
 * Helper routines for decoding x86 architectural state.
 */
#include <xtf/lib.h>
#include <xtf/libc.h>

#include <arch/x86/decode.h>
#include <arch/x86/processor.h>

const char *x86_exc_short_name(unsigned int exc)
{
    static const char *const names[] =
    {
/** @cond */
#define NAME(x) [X86_EXC_ ## x] = "#" #x
        NAME(DE),  NAME(DB), NAME(NMI), NAME(BP),  NAME(OF), NAME(BR),
        NAME(UD),  NAME(NM), NAME(DF),  NAME(CSO), NAME(TS), NAME(NP),
        NAME(SS),  NAME(GP), NAME(PF),  NAME(SPV), NAME(MF), NAME(AC),
        NAME(MC),  NAME(XM), NAME(VE),
#undef NAME
/** @endcond */
    };

    return (exc < ARRAY_SIZE(names) && names[exc]) ? names[exc] : "??";
}

int x86_exc_decode_ec(char *buf, size_t bufsz, unsigned int ev, unsigned int ec)
{
    switch ( ev )
    {
    case X86_EXC_PF:
        return snprintf(buf, bufsz, "%c%c%c%c%c%c",
                        ec & X86_PFEC_PK      ? 'K' : '-',
                        ec & X86_PFEC_INSN    ? 'I' : 'd',
                        ec & X86_PFEC_RSVD    ? 'R' : '-',
                        ec & X86_PFEC_USER    ? 'U' : 's',
                        ec & X86_PFEC_WRITE   ? 'W' : 'r',
                        ec & X86_PFEC_PRESENT ? 'P' : '-');

    case X86_EXC_TS: case X86_EXC_NP: case X86_EXC_SS:
    case X86_EXC_GP: case X86_EXC_AC:
        if ( ec != 0 )
        {
            if ( ec & X86_EC_IDT )
                return snprintf(buf, bufsz,
                                "IDT vec %u%s",
                                ec >> X86_EC_SEL_SHIFT,
                                ec & X86_EC_EXT ? " EXT" : "");
            else
                return snprintf(buf, bufsz,
                                "%cDT sel %#x%s",
                                ec & X86_EC_TI ? 'L' : 'G',
                                ec & X86_EC_SEL_MASK,
                                ec & X86_EC_EXT ? " EXT" : "");
        }

        /* Fallthrough */
    default:
        return snprintf(buf, bufsz, "%04x", ec);
    }
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
