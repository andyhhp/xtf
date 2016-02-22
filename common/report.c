#include <xtf/types.h>
#include <xtf/report.h>
#include <xtf/console.h>

enum test_status {
    STATUS_RUNNING, /**< Test not yet completed.       */
    STATUS_SUCCESS, /**< Test was successful.          */
    STATUS_SKIP,    /**< Test cannot be completed.     */
    STATUS_ERROR,   /**< Issue with the test itself.   */
    STATUS_FAILURE, /**< Issue with the tested matter. */
};

/** Current status of this test. */
static enum test_status status;

/** Whether a warning has occurred. */
static bool warnings;

static const char *status_to_str[] =
{
#define STA(x) [STATUS_ ## x] = #x

    STA(RUNNING),
    STA(SUCCESS),
    STA(SKIP),
    STA(ERROR),
    STA(FAILURE),

#undef STA
};

static void set_status(enum test_status s)
{
    if ( s > status )
        status = s;
}

void xtf_success(const char *fmt, ...)
{
    set_status(STATUS_SUCCESS);

    if ( fmt )
    {
        va_list args;

        va_start(args, fmt);
        vprintk(fmt, args);
        va_end(args);
    }
}

void xtf_warning(const char *fmt, ...)
{
    warnings = true;

    if ( fmt )
    {
        va_list args;

        va_start(args, fmt);
        vprintk(fmt, args);
        va_end(args);
    }
}

void xtf_skip(const char *fmt, ...)
{
    set_status(STATUS_SKIP);

    if ( fmt )
    {
        va_list args;

        va_start(args, fmt);
        vprintk(fmt, args);
        va_end(args);
    }
}

void xtf_error(const char *fmt, ...)
{
    set_status(STATUS_ERROR);

    if ( fmt )
    {
        va_list args;

        va_start(args, fmt);
        vprintk(fmt, args);
        va_end(args);
    }
}

void xtf_failure(const char *fmt, ...)
{
    set_status(STATUS_FAILURE);

    if ( fmt )
    {
        va_list args;

        va_start(args, fmt);
        vprintk(fmt, args);
        va_end(args);
    }
}

void xtf_report_status(void)
{
    if ( status < STATUS_SUCCESS )
        xtf_error("Test did not report a status\n");

    printk("Test result: %s%s\n",
           status_to_str[status],
           (warnings && (status == STATUS_SUCCESS)) ?
           " with warnings" : "");
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
