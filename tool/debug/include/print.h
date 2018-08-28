// -*-Mode: C+;-*-
/**
@file
@brief Declare the debug-printing API for C.
@verbatim
                               Copyright 2013
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
2013/10/31

@par Last commit:
\$Date: 2014-05-15 13:33:40 -0700 (Thu, 15 May 2014) $ by \$Author: panetta $.

@par Credits:
SLAC
*/
#if !defined(TOOL_DEBUG_PRINT_H)
#define      TOOL_DEBUG_PRINT_H

#include <stdarg.h>

#if defined(__cplusplus)
extern "C" {
#endif

  /** @brief Print a string to the console (no formatting). */
  void dbg_prints(const char*);

  /** @brief Format a string and print it to the console. */
  void dbg_printv(const char* fmt, ...);

  /** @brief Format a string and print it to the console. */
  void dbg_vprintv(const char* fmt, va_list args);

  /** @brief I can't go on! */
  void dbg_bugcheck(const char* fmt, ...);

  /** @brief I can't go on! */
  void dbg_vbugcheck(const char* fmt, va_list);

#if defined(__cplusplus)
} /* End of extern "C" */
#endif

#endif
