/**
 * @file arch/x86/decode.c
 *
 * Helper routines for decoding x86 architectural state.
 */
#include <xtf/lib.h>
#include <xtf/libc.h>

#include <arch/decode.h>
#include <arch/processor.h>

const char *x86_vendor_name(enum x86_vendor v)
{
    static const char *const names[] =
    {
        [X86_VENDOR_INTEL] = "Intel",
        [X86_VENDOR_AMD]   = "AMD",
    };

    return (v < ARRAY_SIZE(names) && names[v]) ? names[v] : "Unknown";
}

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
            return snprintf(buf, bufsz,
                            "%cDT[%u]%s",
                            ec & X86_EC_IDT ? 'I' :
                            (ec & X86_EC_TI ? 'L' : 'G'),
                            ec >> X86_EC_SEL_SHIFT,
                            ec & X86_EC_EXT ? ",EXT" : "");
        }

        /* Fallthrough */
    default:
        return snprintf(buf, bufsz, "%04x", ec);
    }
}

int x86_decode_exinfo(char *buf, size_t bufsz, exinfo_t info)
{
    if ( !info )
        return snprintf(buf, bufsz, "nothing");

    unsigned int vec = exinfo_vec(info), ec = exinfo_ec(info);

    if ( ec || ((vec < 32) && ((1u << vec) & X86_EXC_HAVE_EC)) )
    {
        char ecstr[16];

        x86_exc_decode_ec(ecstr, ARRAY_SIZE(ecstr), vec, ec);

        return snprintf(buf, bufsz, "%s[%s]", x86_exc_short_name(vec), ecstr);
    }
    else
        return snprintf(buf, bufsz, "%s", x86_exc_short_name(vec));
}

bool arch_fmt_pointer(
    char **str_ptr, char *end, const char **fmt_ptr, const void *arg,
    int width, int precision, unsigned int flags)
{
    const char *fmt = *fmt_ptr;
    char *str = *str_ptr;

    switch ( fmt[1] )
    {
    case 'e':
    {
        exinfo_t ex = _u(arg);
        char buf[16];

        /* Consumed 'e' from the format string. */
        ++*fmt_ptr;

        x86_decode_exinfo(buf, sizeof buf, ex);

        str = fmt_string(str, end, buf, width, precision, flags);
        break;
    }

    default:
        return false;
    }

    *str_ptr = str;
    return true;
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
