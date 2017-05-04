#include <xtf/libc.h>

int snprintf(char *buf, size_t size, const char *fmt, ...)
{
    va_list args;
    int rc;

    va_start(args, fmt);
    rc = vsnprintf(buf, size, fmt, args);
    va_end(args);

    return rc;
}
