#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

#define vsnprintf xtf_vsnprintf
#include "../common/libc/vsnprintf.c"
#undef vsnprintf

/*
 * To build and run:
 *
 * gcc -I include/ -Wall -Werror -Wextra -Wno-format -O3 -D_FORTIFY_SOURCE=2 test-vsnprintf.c -o test-vsnprintf
 * valgrind --track-origins=yes ./test-vsnprintf
 */

static bool debug = false; /* Always print intermediate buffers? */

static bool __attribute__((format(printf, 3, 4)))
test(char *str, size_t size, const char *fmt, ...)
{
    size_t newsz = size + 2;
    char libstr[newsz + 1];
    char xtfstr[newsz + 1];
    int librc, xtfrc;
    va_list libargs, xtfargs;
    bool ok;

    /* Clobber destination buffers with '$'. */
    memset(libstr, '$', newsz); libstr[newsz] = '\0';
    memset(xtfstr, '$', newsz); xtfstr[newsz] = '\0';

    va_start(libargs, fmt);
    va_copy(xtfargs, libargs);

    /* Format using the library. */
    librc = vsnprintf(str ? libstr : NULL, size, fmt, libargs);
    va_end(libargs);

    /* Format using xtf. */
    xtfrc = xtf_vsnprintf(str ? xtfstr : NULL, size, fmt, xtfargs);
    va_end(xtfargs);

    /* Are the results the same? */
    ok = ( librc == xtfrc &&
           memcmp(libstr, xtfstr, newsz) == 0 );

    if ( debug || !ok )
    {
        printf("\n  lib: '%s', %d\n", libstr, librc);
        printf("  xtf: '%s', %d\n", xtfstr, xtfrc);
    }
    else if ( ok )
        printf("= '%s' - OK\n", xtfstr);

    return ok;
}

/* Stringize the arguments, log them, then test them. */
#define TEST(args...)                          \
    ({ printf("case (%s): ", # args);          \
        if (!test(args)) success = false;      \
    })
#define TEST_NONFATAL(args...)                 \
    ({ printf("case (%s): ", # args);          \
        test(args);                            \
    })

int main(void)
{
    bool success = true;
    char buf[1];

    printf("Testing xtf's vsnprintf() implementation against the local libc\n");

    TEST(NULL, 0, "");
    TEST(NULL, 0, "abc");
    TEST(buf, 0, "abc");
    TEST(buf, 1, "abc");
    TEST(buf, 2, "abc");
    TEST(buf, 3, "abc");
    TEST(buf, 4, "abc");

    TEST(buf, 0, "%%");
    TEST(buf, 1, "%%");
    TEST(buf, 2, "%%");
    TEST(buf, 3, "%%");

    TEST(buf, 0, "%s", NULL);
    TEST(buf, 2, "%s", NULL);
    TEST_NONFATAL(buf, 4, "%s", NULL);
    TEST_NONFATAL(buf, 6, "%s", NULL);

    TEST(buf, 0, "%s", "abc");
    TEST(buf, 2, "%s", "abc");
    TEST(buf, 4, "%s", "abc");
    TEST(buf, 6, "%s", "abc");

    TEST(buf, 30, "%3s", "abcd");
    TEST(buf, 30, "%5s", "abcd");
    TEST(buf, 30, "%7s", "abcd");

    TEST(buf, 30, "%-3s", "abcd");
    TEST(buf, 30, "%-5s", "abcd");
    TEST(buf, 30, "%-7s", "abcd");

    TEST(buf, 30, "%-3.1s", "abcd");
    TEST(buf, 30, "%-5.2s", "abcd");
    TEST(buf, 30, "%-7.3s", "abcd");

    TEST(buf, 30, "%*s", 1, "abcd");
    TEST(buf, 30, "%*s", 5, "abcd");
    TEST(buf, 30, "%*s", -1, "abcd");
    TEST(buf, 30, "%*s", -5, "abcd");

    TEST(buf, 30, "%*.*s", 1, 1, "abcd");
    TEST(buf, 30, "%*.*s", 5, 5, "abcd");
    TEST(buf, 30, "%*.*s", -1, -1, "abcd");
    TEST(buf, 30, "%*.*s", -5, -5, "abcd");

    TEST(buf, 30, "%c", 'a');
    TEST(buf, 30, "%1c", 'a');
    TEST(buf, 30, "%2c", 'a');

    TEST(buf, 30, "%.1c", 'a');
    TEST(buf, 30, "%1.1c", 'a');
    TEST(buf, 30, "%2.2c", 'a');

    TEST(buf, 30, "%-.1c", 'a');
    TEST(buf, 30, "%-1.1c", 'a');
    TEST(buf, 30, "%-2.2c", 'a');

    TEST_NONFATAL(buf, 30, "%-.-1c", 'a');
    TEST_NONFATAL(buf, 30, "%-1.-1c", 'a');
    TEST_NONFATAL(buf, 30, "%-2.-2c", 'a');

    TEST(buf, 10, "%d", 0);
    TEST(buf, 10, "%i", 0);
    TEST(buf, 10, "%o", 0);
    TEST(buf, 10, "%x", 0);
    TEST(buf, 10, "%u", 0);
    TEST(buf, 10, "%X", 0);

    TEST(buf, 10, "%d", 0x1f);
    TEST(buf, 10, "%i", 0x1f);
    TEST(buf, 10, "%o", 0x1f);
    TEST(buf, 10, "%x", 0x1f);
    TEST(buf, 10, "%u", 0x1f);
    TEST(buf, 10, "%X", 0x1f);

    TEST(buf, 30, "%"PRIo8, 0xfedcba9876543210ULL);
    TEST(buf, 30, "%"PRIo16, 0xfedcba9876543210ULL);
    TEST(buf, 30, "%"PRIo32, 0xfedcba9876543210ULL);
    TEST(buf, 30, "%"PRIo64, 0xfedcba9876543210ULL);

    TEST(buf, 30, "%"PRId8, 0xfedcba9876543210ULL);
    TEST(buf, 30, "%"PRId16, 0xfedcba9876543210ULL);
    TEST(buf, 30, "%"PRId32, 0xfedcba9876543210ULL);
    TEST(buf, 30, "%"PRId64, 0xfedcba9876543210ULL);

    TEST(buf, 30, "%"PRIi8, 0xfedcba9876543210ULL);
    TEST(buf, 30, "%"PRIi16, 0xfedcba9876543210ULL);
    TEST(buf, 30, "%"PRIi32, 0xfedcba9876543210ULL);
    TEST(buf, 30, "%"PRIi64, 0xfedcba9876543210ULL);

    TEST(buf, 30, "%"PRIu8, 0xfedcba9876543210ULL);
    TEST(buf, 30, "%"PRIu16, 0xfedcba9876543210ULL);
    TEST(buf, 30, "%"PRIu32, 0xfedcba9876543210ULL);
    TEST(buf, 30, "%"PRIu64, 0xfedcba9876543210ULL);

    TEST(buf, 30, "%"PRIx8, 0xfedcba9876543210ULL);
    TEST(buf, 30, "%"PRIx16, 0xfedcba9876543210ULL);
    TEST(buf, 30, "%"PRIx32, 0xfedcba9876543210ULL);
    TEST(buf, 30, "%"PRIx64, 0xfedcba9876543210ULL);

    TEST(buf, 30, "%"PRIX8, 0xfedcba9876543210ULL);
    TEST(buf, 30, "%"PRIX16, 0xfedcba9876543210ULL);
    TEST(buf, 30, "%"PRIX32, 0xfedcba9876543210ULL);
    TEST(buf, 30, "%"PRIX64, 0xfedcba9876543210ULL);

    TEST(buf, 30, "%hho", 0xfedcba9876543210ULL);
    TEST(buf, 30, "%hhd", 0xfedcba9876543210ULL);
    TEST(buf, 30, "%hhi", 0xfedcba9876543210ULL);
    TEST(buf, 30, "%hhu", 0xfedcba9876543210ULL);
    TEST(buf, 30, "%hhx", 0xfedcba9876543210ULL);
    TEST(buf, 30, "%hhX", 0xfedcba9876543210ULL);

    TEST(buf, 30, "% d", 0);
    TEST(buf, 30, "%-d", 0);
    TEST(buf, 30, "%+d", 0);

    TEST(buf, 30, "% d", 1);
    TEST(buf, 30, "%-d", 1);
    TEST(buf, 30, "%+d", 1);

    TEST(buf, 30, "% d", -1);
    TEST(buf, 30, "%-d", -1);
    TEST(buf, 30, "%+d", -1);

    TEST(buf, 30, "%"PRId64, INT64_MAX);
    TEST(buf, 30, "%"PRId64, INT64_MIN);

    TEST(buf, 30, "%"PRIo64, INT64_MAX);
    TEST(buf, 30, "%"PRIu64, INT64_MAX);
    TEST(buf, 30, "%"PRIx64, INT64_MAX);
    TEST(buf, 30, "%"PRIX64, INT64_MAX);

    TEST(buf, 30, "%d", 2);
    TEST(buf, 30, "%2d", 2);
    TEST(buf, 30, "%2.1d", 2);
    TEST(buf, 30, "%2.3d", 2);

    TEST(buf, 30, "% d", 2);
    TEST(buf, 30, "% 2d", 2);
    TEST(buf, 30, "% 2.1d", 2);
    TEST(buf, 30, "% 2.3d", 2);

    TEST(buf, 30, "%+d", 2);
    TEST(buf, 30, "%+2d", 2);
    TEST(buf, 30, "%+2.1d", 2);
    TEST(buf, 30, "%+2.3d", 2);

    TEST(buf, 30, "%-d", 2);
    TEST(buf, 30, "%-2d", 2);
    TEST(buf, 30, "%-2.1d", 2);
    TEST(buf, 30, "%-2.3d", 2);

    TEST(buf, 30, "%-d", -2);
    TEST(buf, 30, "%-2d", -2);
    TEST(buf, 30, "%-2.1d", -2);
    TEST(buf, 30, "%-2.3d", -2);

    TEST(buf, 30, "%#05x", 0xf);
    TEST(buf, 30, "%# 05x", 0xf);
    TEST(buf, 30, "%#-05x", 0xf);
    TEST(buf, 30, "%#+05x", 0xf);
    TEST(buf, 30, "%#- 5x", 0xf);
    TEST(buf, 30, "%#+ 5x", 0xf);

    TEST(buf, 30, "%#05.2x", 0xf);
    TEST(buf, 30, "%# 05.2x", 0xf);
    TEST(buf, 30, "%#-05.2x", 0xf);
    TEST(buf, 30, "%#+05.2x", 0xf);
    TEST(buf, 30, "%#- 5.2x", 0xf);
    TEST(buf, 30, "%#+ 5.2x", 0xf);

    TEST(buf, 30, "%#05.2x", 0xffff);
    TEST(buf, 30, "%# 05.2x", 0xffff);
    TEST(buf, 30, "%#-05.2x", 0xffff);
    TEST(buf, 30, "%#+05.2x", 0xffff);
    TEST(buf, 30, "%#- 5.2x", 0xffff);
    TEST(buf, 30, "%#+ 5.2x", 0xffff);

    TEST(buf, 30, "%#05.2o", 0xffff);
    TEST(buf, 30, "%# 05.2o", 0xffff);
    TEST(buf, 30, "%#-05.2o", 0xffff);
    TEST(buf, 30, "%#+05.2o", 0xffff);
    TEST(buf, 30, "%#- 5.2o", 0xffff);
    TEST(buf, 30, "%#+ 5.2o", 0xffff);

    TEST(buf, 30, "%#05.2d", 0xffff);
    TEST(buf, 30, "%# 05.2d", 0xffff);
    TEST(buf, 30, "%#-05.2d", 0xffff);
    TEST(buf, 30, "%#+05.2d", 0xffff);
    TEST(buf, 30, "%#- 5.2d", 0xffff);
    TEST(buf, 30, "%#+ 5.2d", 0xffff);

    TEST(buf, 30, "%#05.2zu", 0xffff);
    TEST(buf, 30, "%# 05.2zu", 0xffff);
    TEST(buf, 30, "%#-05.2zu", 0xffff);
    TEST(buf, 30, "%#+05.2zu", 0xffff);
    TEST(buf, 30, "%#- 5.2zu", 0xffff);
    TEST(buf, 30, "%#+ 5.2zu", 0xffff);

    TEST_NONFATAL(buf, 20, "%p", buf);
    TEST_NONFATAL(buf, 20, "%2p", buf);
    TEST(buf, 20, "%#2p", buf);
    TEST(buf, 20, "%#2.0p", buf);
    TEST(buf, 20, "%#2.20p", buf);
    TEST_NONFATAL(buf, 20, "%2.20p", buf);
    TEST(buf, 20, "%#20.20p", buf);
    TEST_NONFATAL(buf, 20, "%20.20p", buf);
    TEST_NONFATAL(buf, 20, "%03p", buf);

    printf("\n%s\n", success ? "SUCCESS" : "FAILED");
    return !success;
}
