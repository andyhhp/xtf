#ifndef XTF_REPORT_H
#define XTF_REPORT_H

#include <xtf/types.h>
#include <xtf/compiler.h>

/**
 * @file include/xtf/report.h
 *
 * API for reporting test status.
 *
 * A test is expected to report one of:
 *  - Success
 *  - Error
 *  - Failure
 *
 * 'Success' indicates that everything went well.  'Error' indicates a bug in
 * the test code or environment itself, while 'Failure' indicates a bug in the
 * code under test.
 *
 * If multiple statuses are reported, the most severe is the one which is
 * kept.
 */

/**
 * Report test success.
 */
void xtf_success(void);

/**
 * Report a test warning.
 *
 * Does not count against a test success, but does indicate that some problem
 * was encountered.
 */
void xtf_warning(const char *fmt, ...) __printf(1, 2);

/**
 * Report a test error.
 *
 * Indicates an error with the test code, or environment, and not with the
 * subject matter under test.
 */
void xtf_error(const char *fmt, ...) __printf(1, 2);

/**
 * Report a test failure.
 *
 * Indicates that the subject matter under test has failed to match
 * expectation.
 */
void xtf_failure(const char *fmt, ...) __printf(1, 2);

/**
 * Print a status report.
 *
 * If a report has not yet been set, an error will occur.
 */
void xtf_report_status(void);

#endif /* XTF_REPORT_H */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
