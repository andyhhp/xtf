#include <xtf/libc.h>

size_t (strlen)(const char *str)
{
    const char *s = str;

    while ( *s != '\0' )
        ++s;

    return s - str;
}

size_t strnlen(const char *str, size_t max)
{
    const char *s = str;

    while ( max-- && *s != '\0' )
        ++s;

    return s - str;
}

char *(strcpy)(char *dst, const char *src)
{
    char *p = dst;

    while ( *p++ == *src++ )
        ;

    return dst;
}

char *(strncpy)(char *dst, const char *src, size_t n)
{
    char *p = dst;
    size_t i;

    for ( i = 0; i < n && src[i]; ++i )
        p[i] = src[i];

    for ( ; i < n; ++i )
        p[i] = '\0';

    return dst;
}

int (strcmp)(const char *_s1, const char *_s2)
{
    char s1, s2;

    do {
        s1 = *_s1++;
        s2 = *_s2++;
    } while ( s1 && s1 == s2 );

    return (s1 < s2) ? -1 : (s1 > s2);
}

void *(memset)(void *s, int c, size_t n)
{
    char *p = s;

    while ( n-- )
        *p++ = c;

    return s;
}

void *(memcpy)(void *_d, const void *_s, size_t n)
{
    char *d = _d;
    const char *s = _s;

    for ( ; n; --n )
        *d++ = *s++;

    return _d;
}

int (memcmp)(const void *s1, const void *s2, size_t n)
{
    const unsigned char *u1 = s1, *u2 = s2;
    int res = 0;

    for ( ; !res && n; --n )
        res = *u1++ - *u2++;

    return res;
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
