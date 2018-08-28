// -*-Mode: C;-*-
/**
@file
@brief Declare functions used to redirect I/O for tasks run from the RTEMS shell.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
DAT

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2014/02/25

@par Last commit:
\$Date: 2014-06-04 17:27:22 -0700 (Wed, 04 Jun 2014) $ by \$Author: panetta $.

@par Credits:
SLAC
*/
#if !defined(SERVICE_SHELL_REDIRECT_H)
#define      SERVICE_SHELL_REDIRECT_H

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <rtems.h>

/* @brief Return the fd for the telnet socket. Must be called from the
   parent task. Returns -1 if the fd would have been 0, 1, or 2;
   these are sacred console fds.
*/
static inline int shell_getTtyFd() {
  /* If this is a shell task then stdin, stdout and stderr are all
     associated with the same socket. We'll choose stdin.
  */
  int fd = fileno(stdin);

  /* Avoid redirection if the parent task is using /dev/console. */
  if (fd <= 3) fd = -1;

  return fd;
}




/* @brief Redirect stdin, stdout and stderr to a telnet socket. Must
   be called from the child task.

   @param[in] fd The fd of the socket, obtained
   by calling shell_getTtyFd() in the parent task.
*/
static inline void shell_redirectStdio(int fd) {
  if (fd < 0) {
    /* No redirection. */
    return;
  }

  /* Trigger the per-task initialization of stdin, stdout and stderr,
   if it hasn't already happened. We don't want it to happen later and
   wipe out our redirection.
  */
  fileno(stdout);

  /* Create streams based on the saved fd. */
  FILE* stream[3], *ostream[3] = {stdin, stdout, stderr};
  int i;
  for (i = 0; i < 3; ++i) {
    if ( !(stream[i] = fdopen(fd, "r+")) ) {
      printk("Error for fdopen: %s\n", strerror(errno));
      return;
    }
  }

  /* Redirect. */
  stdin  = stream[0];
  stdout = stream[1];
  stderr = stream[2];
}
#endif
