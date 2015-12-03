#include <string.h>

size_t strlen(const char *str)
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

void *memset(void *s, int c, size_t n)
{
    char *p = s;

    while ( n-- )
        *p++ = c;

    return s;
}

int memcmp(const void *s1, const void *s2, size_t n)
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
