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

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
