#ifndef __XEN_PUBLIC_ERRNO_H__
#define __XEN_PUBLIC_ERRNO_H__

/*
 * Values originating from x86 Linux. Please consider using respective
 * values when adding new definitions here.
 *
 * The set of identifiers to be added here shouldn't extend beyond what
 * POSIX mandates (see e.g.
 * http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/errno.h.html)
 * with the exception that we support some optional (XSR) values
 * specified there (but no new ones should be added).
 */

#define EPERM           1 /* Operation not permitted */
#define ENOENT          2 /* No such file or directory */
#define ESRCH           3 /* No such process */
#define EINTR           4 /* Interrupted system call */
#define EIO             5 /* I/O error */
#define ENXIO           6 /* No such device or address */
#define E2BIG           7 /* Arg list too long */
#define ENOEXEC         8 /* Exec format error */
#define EBADF           9 /* Bad file number */
#define ECHILD         10 /* No child processes */
#define EAGAIN         11 /* Try again */
#define ENOMEM         12 /* Out of memory */
#define EACCES         13 /* Permission denied */
#define EFAULT         14 /* Bad address */
#define EBUSY          16 /* Device or resource busy */
#define EEXIST         17 /* File exists */
#define EXDEV          18 /* Cross-device link */
#define ENODEV         19 /* No such device */
#define EINVAL         22 /* Invalid argument */
#define ENFILE         23 /* File table overflow */
#define EMFILE         24 /* Too many open files */
#define ENOSPC         28 /* No space left on device */
#define EMLINK         31 /* Too many links */
#define EDOM           33 /* Math argument out of domain of func */
#define ERANGE         34 /* Math result not representable */
#define EDEADLK        35 /* Resource deadlock would occur */
#define ENAMETOOLONG   36 /* File name too long */
#define ENOLCK         37 /* No record locks available */
#define ENOSYS         38 /* Function not implemented */
#define ENODATA        61 /* No data available */
#define ETIME          62 /* Timer expired */
#define EBADMSG        74 /* Not a data message */
#define EOVERFLOW      75 /* Value too large for defined data type */
#define EILSEQ         84 /* Illegal byte sequence */
#define ERESTART       85 /* Interrupted system call should be restarted */
#define ENOTSOCK       88 /* Socket operation on non-socket */
#define EOPNOTSUPP     95 /* Operation not supported on transport endpoint */
#define EADDRINUSE     98 /* Address already in use */
#define EADDRNOTAVAIL  99 /* Cannot assign requested address */
#define ENOBUFS       105 /* No buffer space available */
#define EISCONN       106 /* Transport endpoint is already connected */
#define ENOTCONN      107 /* Transport endpoint is not connected */
#define ETIMEDOUT     110 /* Connection timed out */

#endif /*  __XEN_PUBLIC_ERRNO_H__ */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
