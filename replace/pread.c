/* $Id$

   Replacement for a missing pread.

   Written by Russ Allbery <rra@stanford.edu>
   This work is hereby placed in the public domain by its author.

   %AUTOCONF%
   AC_CHECK_HEADERS(unistd.h)
   AC_TYPE_OFF_T
   AC_CHECK_TYPE(ssize_t, int)
   %%

   Provides the same functionality as the standard library routine pread
   for those platforms that don't have it.  Note that pread requires that
   the file pointer not move and without the library function, we can't copy
   that behavior; instead, we approximate it by moving the file pointer and
   then moving it back.  This may break threaded programs. */

#include "config.h"

#include <errno.h>
#include <sys/types.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifndef SEEK_SET
# define SEEK_SET 0
#endif
#ifndef SEEK_CUR
# define SEEK_CUR 1
#endif

/* If we're running the test suite, rename pread to avoid conflicts with the
   system version. */
#if TESTING
# define pread test_pread
#endif

ssize_t
pread(int fd, void *buf, size_t nbyte, off_t offset)
{
    off_t current;
    ssize_t nread;
    int oerrno;

    current = lseek(fd, 0, SEEK_CUR);
    if (current == (off_t) -1 || lseek(fd, offset, SEEK_SET) == (off_t) -1)
        return -1;

    nread = read(fd, buf, nbyte);

    /* Ignore errors in restoring the file position; this isn't ideal, but
       reporting a failed read when the read succeeded is worse.  Make sure
       that errno, if set, is set by read and not lseek. */
    oerrno = errno;
    lseek(fd, current, SEEK_SET);
    errno = oerrno;
    return nread;
}
