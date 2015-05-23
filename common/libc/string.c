#include <string.h>

size_t strlen(const char *str)
{
    const char *s = str;

    while ( *s != '\0' )
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
