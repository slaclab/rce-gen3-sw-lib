// -*-Mode: C++;-*-
/**
@file
@brief Declare the private class dbg_SyslogTracker for RTEMS.
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
2014/04/16

@par Last commit:
\$Date: 2014-07-31 17:05:57 -0700 (Thu, 31 Jul 2014) $ by \$Author: tether $.

@par Revision number:
\$Revision: 3597 $

@par Credits:
SLAC
*/
#if !defined(TOOL_DEBUG_SYSLOGTRACKER_HH)
#define      TOOL_DEBUG_SYSLOGTRACKER_HH

#include <rtems.h>

#if defined(__cplusplus)
extern "C" {
#endif

  /* The states of the syslog buffer. */
  typedef enum {
    BUFFER_EMPTY,    // Empty.
    BUFFER_FILLING,  // Not empty, the write pointer has not wrapped around yet.
    BUFFER_FULL      // Filled up. The write pointer has or is about to wrap around.
  } BufferState;

  /* The states of the current line being written to the syslog buffer. */
  typedef enum {
    LINE_EMPTY,     // The current line has no content, not even '\n'.
    LINE_NONEMPTY   // Line not empty.
  } LineState;
  

  /*  Wrap the character-output routine provided by the BSP
      so that we can capture the output in a circular buffer
      and add timestamps to the beginning of each line.

      The function dbg_initSyslog() creates an instance of this class
      and replaces BSP_output_char with a pointer to
      a function that calls dbg_SyslogTracker_sendChar() on the instance.
  */
  typedef struct {
    char*       bufferAddress;
    int         bufferSize;
    int         inext;   // The write pointer (index).
    BufferState bufferState;
    LineState   lineState;
  } dbg_SyslogTracker;

  /** Initialize a new tracker.
      buffadd   The address of the BSP message buffer.
      buffsize  The size of the BSP message buffer.
*/
  void dbg_SyslogTracker_construct
  (
   dbg_SyslogTracker* self,
   char* buffadd,
   int buffsize
   );

  void dbg_SyslogTracker_destroy(dbg_SyslogTracker* self);

  void dbg_SyslogTracker_capture(dbg_SyslogTracker* self, char ch);

  void dbg_SyslogTracker_stamp(dbg_SyslogTracker* self, char ch);

#if defined(__cplusplus)
} /* extern "C" */
#endif
#endif
