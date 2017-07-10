#include <arch/div.h>
#include <xtf/libc.h>
#include <xtf/compiler.h>

#ifndef isdigit
/* Avoid pulling in all of ctypes just for this. */
static int isdigit(int c)
{
    return c >= '0' && c <= '9';
}
#endif

/*
 * The subset of formatting supported:
 *
 * From the C11 specification:
 * A conversion specification is the '%' character followed by:
 * - Zero or more flags: ('-', '+', ' ', '#', '0')
 *   - '-' Left justified
 *   - '+' Explicit sign
 *   - ' ' Space instead of sign
 *   - '#' Alternative representation
 *   - '0' Zero-pad from the left
 * - Optional minimum width field ('*' or integer)
 * - Optional precision ('.' followed by '*' or integer)
 * - Optional length modifier:
 *   - 'hh' - char
 *   - 'h'  - short
 *   - 'l'  - long
 *   - 'll' - long long
 *   - 'z'  - size_t
 * - Mandatory coversion specifier:
 *   - signed integer ('d', 'i')
 *   - unsigned integer ('o', 'u', 'x', 'X' uppercase)
 *   - unsigned char ('c')
 *   - array of char ('s')
 *   - pointer to void ('p')
 *   - literal '%'
 */

/* Flags */
#define LEFT      (1u << 0)
#define PLUS      (1u << 1)
#define SPACE     (1u << 2)
#define ALTERNATE (1u << 3)
#define ZERO      (1u << 4)
/* Conversions */
#define UPPER     (1u << 5)
#define SIGNED    (1u << 6)

/* Shorthand for ensuring str moves forwards, but not overruning the buffer. */
#define PUT(c)                                  \
    ({ if ( str < end )                         \
            *str = (c);                         \
        ++str;                                  \
    })

static int fmt_int(const char **fmt)
{
    int res = 0;

    while( isdigit(**fmt) )
    {
        res *= 10;
        res += **fmt - '0';
        ++*fmt;
    }

    return res;
}

char *fmt_number(char *str, char *end, long long val, unsigned int base,
                 int width, int precision, unsigned int flags)
{
    static const char lower[] = "0123456789abcdef";
    static const char upper[] = "0123456789ABCDEF";
    const char *digits = (flags & UPPER) ? upper : lower;

    char tmp[24], prefix = '\0';
    int i = 0;
    uint64_t uval = val;

    /* Sanity check base. */
    if ( !(base == 8 || base == 10 || base == 16) )
        return str;

    /* Override zeropad if we are aligning left or have a specific precsion. */
    if ( (flags & LEFT) || (precision != -1) )
        flags &= ~ZERO;

    /* Signed values only can get explicit svign treatment. */
    if ( flags & SIGNED )
    {
        if ( val < 0 )
        {
            prefix = '-';
            uval = -val;
            width--;
        }
        else if ( flags & PLUS )
        {
            prefix = '+';
            width--;
        }
        else if ( flags & SPACE )
        {
            prefix = ' ';
            width--;
        }
    }

    /* Alternate representation applies to oct/hex only. */
    if ( flags & ALTERNATE )
        switch ( base )
        {
        case 16: width -= 2; break;
        case 8:  width -= 1; break;
        }

    /* Make sure we at least have a single '0'. */
    if ( val == 0 )
        tmp[i++] = '0';
    else
        /* tmp contains the number formatted backwards. */
        while ( uval )
            tmp[i++] = digits[divmod64(&uval, base)];

    /* Expand precision if the number is too long. */
    if ( i > precision )
        precision = i;
    width -= precision;

    /* If we are doing nothing special, pad with ' ' on the LHS. */
    if ( (flags & (LEFT|ZERO)) == 0 )
        while ( width-- > 0 )
            PUT(' ');

    /* Optional sign character for signed numbers. */
    if ( prefix )
        PUT(prefix);

    /* Optional leading '0x' or '0'. */
    if ( flags & ALTERNATE )
        switch ( base )
        {
        case 16: PUT('0'); PUT('x'); break;
        case 8:  PUT('0'); break;
        }

    /* Zero pad at the start of the number. */
    if ( flags & ZERO )
        while ( width-- > 0 )
            PUT('0');

    /* If we have too fewer digits than precision, zero pad some more. */
    while ( i < precision-- )
        PUT('0');

    /* Copy the number from tmp. */
    while ( i-- )
        PUT(tmp[i]);

    /* Pad any remaining width on the RHS. */
    while (width-- > 0)
        PUT(' ');

    return str;
}

char *fmt_string(char *str, char *end, const char *val,
                 int width, int precision, unsigned int flags)
{
    int len, i;

    if ( !val )
        val = "(NULL)";

    if ( precision < 0 )
        len = strlen(val);
    else
        len = strnlen(val, precision);

    if ( !(flags & LEFT) )
        while ( len < width-- )
            PUT(' ');

    for ( i = 0; i < len; ++i )
        PUT(val[i]);

    while ( len < width-- )
        PUT(' ');

    return str;
}

static char *pointer(
    char *str, char *end, const char **fmt_ptr, const void *arg,
    int width, int precision, unsigned int flags)
{
    const char *fmt = *fmt_ptr;

    switch ( fmt[1] )
    {
    case 'h': /* Raw buffer as hex string. */
    {
        const uint8_t *hex_buffer = arg;
        char sep = ' ';

        /* Consumed 'h' from the format string. */
        ++*fmt_ptr;

        /* Bound user count from %* to between 0 and 128 bytes. */
        if ( width <= 0 )
            return str;
        if ( width > 128 )
            width = 128;

        /*
         * Peek ahead in the format string to see if a recognised separator
         * modifier is present.
         */
        switch ( fmt[2] )
        {
        case 'C': /* Colons. */
            ++*fmt_ptr;
            sep = ':';
            break;

        case 'D': /* Dashes. */
            ++*fmt_ptr;
            sep = '-';
            break;

        case 'N': /* No separator. */
            ++*fmt_ptr;
            sep = 0;
            break;
        }

        for ( int i = 0; ; )
        {
            /* Each byte: 2 chars, 0-padded, base 16, no hex prefix. */
            str = fmt_number(str, end, hex_buffer[i], 16, 2, -1, ZERO);

            if ( ++i == width )
                return str;

            if ( sep )
                PUT(sep);
        }
    }
    break;

    default:
        if ( arch_fmt_pointer(&str, end, fmt_ptr,
                              arg, width, precision, flags) )
            return str;
    }

    /* Fall back to plain 32/64bit hex integer. */
    if ( width == -1 )
    {
        width = 2 * sizeof(arg);
        flags |= ZERO;
    }

    return fmt_number(str, end, (unsigned long)arg, 16,
                      width, precision, flags);
}

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
    char *str = buf, *end = buf + size;

    for ( ; *fmt != '\0'; ++fmt )
    {
        const char *spec_start = fmt; /* For rewinding on error. */

        unsigned long long num;
        unsigned int base, flags = 0;
        int width = -1, precision = -1;
        char length_mod = 'i';

        /* Put regular characters into the destination. */
        if ( *fmt != '%' )
        {
            PUT(*fmt);
            continue;
        }

 next_flag: /* Process any flags. */
        ++fmt;
        switch ( *fmt )
        {
        case '-': flags |= LEFT;      goto next_flag;
        case '+': flags |= PLUS;      goto next_flag;
        case ' ': flags |= SPACE;     goto next_flag;
        case '#': flags |= ALTERNATE; goto next_flag;
        case '0': flags |= ZERO;      goto next_flag;
        }

        /* Process the width field. */
        if ( *fmt == '*' )
        {
            ++fmt;
            width = va_arg(args, int);
            if ( width < 0 )
            {
                flags |= LEFT;
                width = -width;
            }
        }
        else if ( isdigit(*fmt) )
        {
            width = fmt_int(&fmt);
            if ( width < 0 )
            {
                flags |= LEFT;
                width = -width;
            }
        }

        /* Process the precision field. */
        if ( *fmt == '.' )
        {
            ++fmt;
            if ( *fmt == '*' )
            {
                ++fmt;
                precision = va_arg(args, int);
            }
            else if ( isdigit(*fmt) )
                precision = fmt_int(&fmt);

            /* Negative precision is meaningless */
            if ( precision < 0 )
                precision = -1;
        }

        /* Process the length modifier. */
        switch ( *fmt )
        {
        case 'h': length_mod = 'h'; ++fmt; break;
        case 'l': length_mod = 'l'; ++fmt; break;
        case 'z': length_mod = 'z'; ++fmt; break;
        }
        /* Might be two... */
        switch ( *fmt )
        {
        case 'h': length_mod = 'H'; ++fmt; break;
        case 'l': length_mod = 'L'; ++fmt; break;
        }

        /* Process the conversion modifier. */
        switch ( *fmt )
        {
        case '%': /* Literal '%'. */
            PUT('%');
            continue;

        case 'c': /* Unsigned char. */
        {
            unsigned char c = va_arg(args, int);

            if ( !(flags & LEFT) )
                while ( --width > 0 )
                    PUT(' ');

            PUT(c);

            while ( --width > 0 )
                PUT(' ');

            continue;
        }

        case 's': /* String. */
            str = fmt_string(str, end, va_arg(args, const char *),
                             width, precision, flags);
            continue;

        case 'p': /* Pointer. */
            str = pointer(str, end, &fmt, va_arg(args, const void *),
                          width, precision, flags);
            continue;

        default: /* Something unrecognised - print the specifier literally. */
            PUT('%');
            fmt = spec_start;
            continue;

        /* From here on down, all the numbers. */

        case 'o': /* Octal. */
            base = 8;
            break;

        case 'd': case 'i': /* Signed decimal. */
            flags |= SIGNED;
            /* fallthrough */
        case 'u': /* Unsigned decimal. */
            base = 10;
            break;

        case 'X': /* Uppercase hex. */
            flags |= UPPER;
            /* fallthrough */
        case 'x': /* Lowercase hex. */
            base = 16;
            break;
        }

        /* Pull the arg and cast correctly. */
        switch ( length_mod )
        {
        case 'H':
            if ( flags & SIGNED )
                num = (signed char)va_arg(args, int);
            else
                num = (unsigned char)va_arg(args, int);
            break;

        case 'h':
            if ( flags & SIGNED )
                num = (signed short)va_arg(args, int);
            else
                num = (unsigned short)va_arg(args, int);
            break;

        case 'i':
            if ( flags & SIGNED )
                num = (signed int)va_arg(args, int);
            else
                num = (unsigned int)va_arg(args, int);
            break;

        case 'z':
            num = (size_t)va_arg(args, size_t);
            break;

        case 'l':
            if ( flags & SIGNED )
                num = (signed long)va_arg(args, long);
            else
                num = (unsigned long)va_arg(args, unsigned long);
            break;

        case 'L':
            if ( flags & SIGNED )
                num = (signed long long)va_arg(args, long long);
            else
                num = (unsigned long long)va_arg(args, unsigned long long);
            break;

        default: /* Really shouldn't happen, but rewind just in case. */
            PUT('%');
            fmt = spec_start;
            continue;
        }

        str = fmt_number(str, end, num, base, width, precision, flags);
    }

    /* NUL terminate the buffer, if there is room (but don't count '\0'). */
    if ( str < end )
        *str = '\0';
    /*
     * Or trucate at the final character if an overrun occurred and buf is not
     * 0 length.
     */
    else if ( size > 0 )
        end[-1] = '\0';

    return str - buf;
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
